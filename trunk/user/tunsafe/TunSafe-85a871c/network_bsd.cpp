// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "network_bsd.h"
#include "network_common.h"
#include "tunsafe_endian.h"
#include "util.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <poll.h>

#if defined(OS_LINUX)
#include <sys/inotify.h>
#include <limits.h>
#include <sys/prctl.h>
#endif

#include <algorithm>

#include "wireguard.h"
#include "wireguard_config.h"

#if defined(OS_MACOSX) || defined(OS_FREEBSD)
#define TUN_PREFIX_BYTES 4
#elif defined(OS_LINUX) || defined(OS_ANDROID)
#define TUN_PREFIX_BYTES 0
#endif

static Packet *freelist;

void tunsafe_die(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

void SetThreadName(const char *name) {
#if defined(OS_LINUX)
  prctl(PR_SET_NAME, name, 0, 0, 0);
#endif  // defined(OS_LINUX)
}

void FreePacket(Packet *packet) {
  packet->queue_next = freelist;
  freelist = packet;
}

Packet *AllocPacket() {
  Packet *p = freelist;
  if (p) {
    freelist = Packet_NEXT(p);
  } else {
    p = (Packet*)malloc(kPacketAllocSize);  
    if (p == NULL) {
      RERROR("Allocation failure");
      abort();
    }
  }
  p->Reset();
  return p;
}

void FreePacketList(Packet *packet) {
  while (packet)
    free(exch(packet, Packet_NEXT(packet)));
}

void FreeAllPackets() {
  FreePacketList(exch_null(freelist));
}

//////////////////////////////////////////////////////////////////////////////////////////////

NetworkBsd::NetworkBsd(NetworkBsdDelegate *delegate, int max_sockets)
    : exit_(false),
      overload_(false),
      sigalarm_flag_(false),
      num_roundrobin_(0),
      num_sock_(0),
      num_endloop_(0),
      read_packet_(NULL),
      tcp_sockets_(NULL),
      delegate_(delegate),
      max_sockets_(max_sockets) {
  if (max_sockets < 5 || max_sockets > 1000)
    tunsafe_die("invalid value for max_sockets");

  pollfd_ = new struct pollfd[max_sockets];
  sockets_ = new BaseSocketBsd*[max_sockets];
  roundrobin_ = new BaseSocketBsd*[max_sockets];
  endloop_ = new BaseSocketBsd*[max_sockets];
  if (!pollfd_ || !sockets_ || !roundrobin_ || !endloop_)
    tunsafe_die("no memory");

  memset(iov_packets_, 0, sizeof(iov_packets_));
}

NetworkBsd::~NetworkBsd() {
  assert(tcp_sockets_ == NULL);
  assert(num_sock_ == 0);
  if (read_packet_)
    FreePacket(read_packet_);
  for (size_t i = 0; i < kMaxIovec; i++)
    if (iov_packets_[i])
      FreePacket(iov_packets_[i]);

  delete [] pollfd_;
  delete [] sockets_;
  delete [] roundrobin_;
  delete [] endloop_;
}

void NetworkBsd::RunLoop(const sigset_t *sigmask) {
  int free_packet_interval = 10;
  int overload_ctr = 0;
  uint64 last_second_loop = 0;
  uint64 now = 0;

  if (!WithSigalarmSupport)
    last_second_loop = OsGetMilliseconds();
  
  while (!exit_) {
    int n;
    bool new_second = false;

    if (WithSigalarmSupport) {
      if (sigalarm_flag_) {
        sigalarm_flag_ = false;
        new_second = true;
      }
    } else {
      now = OsGetMilliseconds();
      if ((now - last_second_loop) >= 1000) {
        // Avoid falling behind too much
        last_second_loop = (now - last_second_loop) >= 2000 ? now : last_second_loop + 1000;
        new_second = true;
      }
    }

    if (new_second) {
      delegate_->OnSecondLoop(now);
      
      struct BaseSocketBsd **socks = sockets_;
      for (int i = 0; i < num_sock_; i++)
        socks[i]->Periodic();

      if (free_packet_interval == 0) {
        FreeAllPackets();
        free_packet_interval = 10;
      }
      free_packet_interval--;

      overload_ctr -= (overload_ctr != 0);
    }

#if defined(OS_LINUX) || defined(OS_FREEBSD)
    n = ppoll(pollfd_, num_sock_, NULL, sigmask);
#else
    n = poll(pollfd_, num_sock_, WithSigalarmSupport ? -1 : std::max<int>((int)(last_second_loop - now) + 1000, 0));
#endif
    if (n == -1) {
      if (errno != EINTR) {
        RERROR("poll failed");
        break;
      }
    } else {
      // Iterate backwards to support deleting elements
      struct pollfd *pfd = pollfd_;
      struct BaseSocketBsd **socks = sockets_;
      for (int i = num_sock_ - 1; i >= 0; i--) {
        if (pfd[i].revents)
          socks[i]->HandleEvents(pfd[i].revents);
      }
    }

    overload_ = (overload_ctr != 0);
    for (int loop = 0; ; loop++) {
      // Whenever we don't finish set overload ctr.
      if (loop == 256) {
        overload_ctr = 4;
        break;
      }
      int i = num_roundrobin_ - 1;
      struct BaseSocketBsd **rrlist = roundrobin_;
      if (i < 0)
        break;
      do {
        if (!rrlist[i]->DoRoundRobin())
          RemoveFromRoundRobin(i);
      } while (i--);
    }

    struct BaseSocketBsd **endloop = endloop_;
    for (int j = num_endloop_ - 1; j >= 0; j--) {
      endloop[j]->endloop_slot_ = -1;
      endloop[j]->DoEndloop();
    }
    num_endloop_ = 0;

    delegate_->RunAllMainThreadScheduled();
  }
}

void NetworkBsd::RemoveFromRoundRobin(int i) {
  BaseSocketBsd *cur = roundrobin_[i], *last = roundrobin_[num_roundrobin_-- - 1];
  assert(cur->roundrobin_slot_ == i);
  roundrobin_[i] = last;
  last->roundrobin_slot_ = i;
  cur->roundrobin_slot_ = -1;
}

void NetworkBsd::ReallocateIov(size_t j) {
  Packet *p = AllocPacket();
  iov_packets_[j] = p;
  iov_[j].iov_base = p->data;
  iov_[j].iov_len = kPacketCapacity;
}

void NetworkBsd::EnsureIovAllocated() {
  if (iov_packets_[0] == NULL) {
    for (size_t i = 0; i < kMaxIovec; i++)
      ReallocateIov(i);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

BaseSocketBsd::~BaseSocketBsd() {
  CloseSocket();
}

void BaseSocketBsd::CloseSocket() {
  if (fd_ != -1)
    close(fd_);
  if (roundrobin_slot_ >= 0)
    network_->RemoveFromRoundRobin(roundrobin_slot_);
  if (endloop_slot_ >= 0) {
    BaseSocketBsd *last = network_->endloop_[network_->num_endloop_-- - 1];
    network_->endloop_[endloop_slot_] = last;
    last->endloop_slot_ = endloop_slot_;
  }
  if (pollfd_slot_ >= 0) {
    unsigned int cur = pollfd_slot_, last = network_->num_sock_-- - 1;
    BaseSocketBsd *lastsock = network_->sockets_[last];
    network_->sockets_[cur] = lastsock;
    lastsock->pollfd_slot_ = cur;
    network_->pollfd_[cur] = network_->pollfd_[last];
  }
  fd_ = -1;
  endloop_slot_ = pollfd_slot_ = roundrobin_slot_ = -1;
}

void BaseSocketBsd::InitPollSlot(int fd, int events) {
  assert(network_->num_sock_ != network_->max_sockets_);
  assert(fd_ == -1);
  fd_ = fd;
  unsigned int slot = pollfd_slot_;
  if (pollfd_slot_ < 0)
    pollfd_slot_ = slot = network_->num_sock_++;
  network_->sockets_[slot] = this;
  struct pollfd *pfd = &network_->pollfd_[slot];
  pfd->fd = fd;
  pfd->events = events;
  pfd->revents = 0;
}

void BaseSocketBsd::AddToRoundRobin() {
  if (roundrobin_slot_ < 0)
    network_->roundrobin_[roundrobin_slot_ = network_->num_roundrobin_++] = this;
}

void BaseSocketBsd::AddToEndLoop() {
  if (endloop_slot_ < 0)
    network_->endloop_[endloop_slot_ = network_->num_endloop_++] = this;
}

int BaseSocketBsd::StealFd() {
  int fd = exch(fd_, -1);
  CloseSocket();
  return fd;
}

//////////////////////////////////////////////////////////////////////////////////////////////

TunSocketBsd::TunSocketBsd(NetworkBsd *network, WireguardProcessor *processor)
    : BaseSocketBsd(network),
      tun_readable_(false),
      tun_writable_(false),
      tun_interface_gone_(false),
      tun_queue_(NULL),
      tun_queue_end_(&tun_queue_),
      processor_(processor) {
}

TunSocketBsd::~TunSocketBsd() {
}

bool TunSocketBsd::Initialize(int fd) {
  if (!HasFreePollSlot())
    return false;
  fcntl(fd, F_SETFD, FD_CLOEXEC);
  fcntl(fd, F_SETFL, O_NONBLOCK);
  InitPollSlot(fd, POLLIN);
  tun_writable_ = true;
  return true;
}

static inline bool IsCompatibleProto(uint32 v) {
  return v == AF_INET || v == AF_INET6;
}

void TunSocketBsd::HandleEvents(int revents) {
  if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
    if (revents & POLLERR) {
      tun_interface_gone_ = true;
      RERROR("Tun interface gone, closing.");
    } else {
      RERROR("Tun interface error %d, closing.", revents);
    }
    tun_readable_ = tun_writable_ = false;
    network_->PostExit();
  } else {
    tun_readable_ = (revents & POLLIN) != 0;
    if (revents & POLLOUT) {
      SetPollFlags(POLLIN);
      tun_writable_ = true;
    }
  }
  AddToRoundRobin();
}

bool TunSocketBsd::DoRead() {
  assert(tun_readable_);
  Packet *packet = network_->read_packet_;
  if (!packet)
    network_->read_packet_ = packet = AllocPacket();

  int r = read(fd_, packet->data - TUN_PREFIX_BYTES, kPacketCapacity + TUN_PREFIX_BYTES);
  if (r >= 0) {
//        printf("Read %d bytes from TUN\n", r);
    packet->size = r - TUN_PREFIX_BYTES;
    if (r >= TUN_PREFIX_BYTES && (!TUN_PREFIX_BYTES || IsCompatibleProto(ReadBE32(packet->data - TUN_PREFIX_BYTES)))) {
      //      printf("%X %X %X %X %X %X %X %X\n",
      //        read_packet_->data[0], read_packet_->data[1], read_packet_->data[2], read_packet_->data[3], 
      //        read_packet_->data[4], read_packet_->data[5], read_packet_->data[6], read_packet_->data[7]);
      network_->read_packet_ = NULL;
      processor_->HandleTunPacket(packet);
    }
    return true;
  } else {
    if (errno != EAGAIN) {
      fprintf(stderr, "Read from tun failed\n");
    }
    tun_readable_ = false;
    return false;
  }
}

static uint32 GetProtoFromPacket(const uint8 *data, size_t size) {
  return size < 1 || (data[0] >> 4) != 6 ? AF_INET : AF_INET6;
}

bool TunSocketBsd::DoWrite() {
  assert(tun_writable_);
  if (TUN_PREFIX_BYTES) {
    WriteBE32(tun_queue_->data - TUN_PREFIX_BYTES, GetProtoFromPacket(tun_queue_->data, tun_queue_->size));
  }
  int r = write(fd_, tun_queue_->data - TUN_PREFIX_BYTES, tun_queue_->size + TUN_PREFIX_BYTES);
  if (r < 0) {
    if (errno == EAGAIN) {
      tun_writable_ = false;
      SetPollFlags(POLLIN | POLLOUT);
      return false;
    }
    RERROR("Write to tun failed");
  } else {
    r -= TUN_PREFIX_BYTES;
    if (r != tun_queue_->size)
      RERROR("Write to tun incomplete!");
    //    else
    //      RINFO("Wrote %d bytes to TUN", r);
  }
  Packet *next = Packet_NEXT(tun_queue_);
  FreePacket(tun_queue_);
  if ((tun_queue_ = next) != NULL) return true;
  tun_queue_end_ = &tun_queue_;
  return false;
}

void TunSocketBsd::WritePacket(Packet *packet) {
  assert(fd_ >= 0);
  Packet *queue_is_used = tun_queue_;
  *tun_queue_end_ = packet;
  tun_queue_end_ = &Packet_NEXT(packet);
  packet->queue_next = NULL;
  if (!queue_is_used)
    DoWrite();
}

bool TunSocketBsd::DoRoundRobin() {
  bool more_work = false;
  if (tun_queue_ && tun_writable_)
    more_work = DoWrite();
  if (tun_readable_)
    more_work |= DoRead();
  return more_work;
}

//////////////////////////////////////////////////////////////////////////////////////////////

UdpSocketBsd::UdpSocketBsd(NetworkBsd *network, WireguardProcessor *processor)
    : BaseSocketBsd(network),
      udp_readable_(false),
      udp_writable_(false),
      udp_queue_(NULL),
      udp_queue_end_(&udp_queue_),
      processor_(processor) {
}

UdpSocketBsd::~UdpSocketBsd() {
}

bool UdpSocketBsd::Initialize(int listen_port) {
  if (!HasFreePollSlot()) {
    RERROR("No free internal sockets");
    return false;
  }
  int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_fd < 0) {
    RERROR("socket(SOCK_DGRAM) failed");  
    return false;
  }
  sockaddr_in sin = {0};
  sin.sin_family = AF_INET;
  sin.sin_port = htons(listen_port);
  if (bind(udp_fd, (struct sockaddr*)&sin, sizeof(sin)) != 0) {
    close(udp_fd);
    RERROR("bind on udp socket port %d failed", listen_port);
    return false;
  }
  fcntl(udp_fd, F_SETFD, FD_CLOEXEC);
  fcntl(udp_fd, F_SETFL, O_NONBLOCK);
  InitPollSlot(udp_fd, POLLIN);
  udp_writable_ = true;
  return true;
}

void UdpSocketBsd::HandleEvents(int revents) {
  if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
    RERROR("UDP error %d, closing.", revents);
    network_->PostExit();
  } else {
    udp_readable_ = (revents & POLLIN) != 0;
    if (revents & POLLOUT) {
      SetPollFlags(POLLIN);
      udp_writable_ = true;
    }
  }
  AddToRoundRobin();
}

bool UdpSocketBsd::DoRead() {
  socklen_t sin_len;
  Packet *read_packet = network_->read_packet_;
  if (read_packet == NULL)
    network_->read_packet_ = read_packet = AllocPacket();

  sin_len = sizeof(read_packet->addr.sin);
  int r = recvfrom(fd_, read_packet->data, kPacketCapacity, 0,
                   (sockaddr*)&read_packet->addr.sin, &sin_len);
  if (r >= 0) {
    //    printf("Read %d bytes from UDP\n", r);
    read_packet->sin_size = sin_len;
    read_packet->size = r;
    read_packet->protocol = kPacketProtocolUdp;
    network_->read_packet_ = NULL;

    if (processor_->dev().packet_obfuscator().enabled())
      processor_->dev().packet_obfuscator().DeobfuscatePacket(read_packet);
    processor_->HandleUdpPacket(read_packet, network_->overload_);
    return true;
  } else {
    if (errno != EAGAIN) {
      fprintf(stderr, "Read from UDP failed\n");
    }
    udp_readable_ = false;
    return false;
  }
}

bool UdpSocketBsd::DoWrite() {
  assert(udp_writable_);
  //  RINFO("Send %d bytes to %s", (int)udp_queue_->size, inet_ntoa(udp_queue_->sin.sin_addr));
  int r = sendto(fd_, udp_queue_->data, udp_queue_->size, 0,
                 (sockaddr*)&udp_queue_->addr.sin, sizeof(udp_queue_->addr.sin));
  if (r < 0) {
    if (errno == EAGAIN) {
      udp_writable_ = false;
      SetPollFlags(POLLIN | POLLOUT);
      return false;
    }
    perror("Write to UDP failed");
  } else {
    if (r != udp_queue_->size)
      perror("Write to udp incomplete!");
    //    else
    //      RINFO("Wrote %d bytes to UDP", r);
  }
  Packet *next = Packet_NEXT(udp_queue_);
  FreePacket(udp_queue_);
  if ((udp_queue_ = next) != NULL) return true;
  udp_queue_end_ = &udp_queue_;
  return false;
}

void UdpSocketBsd::WritePacket(Packet *packet) {
  assert(fd_ >= 0);

  if (processor_->dev().packet_obfuscator().enabled())
    processor_->dev().packet_obfuscator().ObfuscatePacket(packet);
   
  Packet *queue_is_used = udp_queue_;
  *udp_queue_end_ = packet;
  udp_queue_end_ = &Packet_NEXT(packet);
  packet->queue_next = NULL;
  if (!queue_is_used)
    DoWrite();
}

bool UdpSocketBsd::DoRoundRobin() {
  bool did_work = false;
  if (udp_queue_ && udp_writable_)
    did_work = DoWrite();
  if (udp_readable_)
    did_work |= DoRead();
  return did_work;
}

//////////////////////////////////////////////////////////////////////////////////////////////

#if defined(OS_LINUX)
UnixSocketDeletionWatcher::UnixSocketDeletionWatcher() 
    : inotify_fd_(-1) {
  pipes_[0] = -1;
  pipes_[0] = -1;
}

UnixSocketDeletionWatcher::~UnixSocketDeletionWatcher() {
  close(inotify_fd_);
  close(pipes_[0]);
  close(pipes_[1]);
}

bool UnixSocketDeletionWatcher::Start(const char *path, bool *flag_to_set) {
  assert(inotify_fd_ == -1);
  path_ = path;
  flag_to_set_ = flag_to_set;
  pid_ = getpid();
  inotify_fd_ = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
  if (inotify_fd_ == -1) {
    perror("inotify_init1() failed");
    return false;
  }
  if (inotify_add_watch(inotify_fd_, "/var/run/wireguard", IN_DELETE | IN_DELETE_SELF) == -1) {
    perror("inotify_add_watch failed");
    return false;
  }
  if (pipe(pipes_) == -1) {
    perror("pipe() failed");
    return false;
  }
  return pthread_create(&thread_, NULL, &UnixSocketDeletionWatcher::RunThread, this) == 0;
}

void UnixSocketDeletionWatcher::Stop() {
  RINFO("Stopping..");
  void *retval;
  write(pipes_[1], "", 1);
  pthread_join(thread_, &retval);
}

void *UnixSocketDeletionWatcher::RunThread(void *arg) {
  UnixSocketDeletionWatcher *self = (UnixSocketDeletionWatcher*)arg;
  return self->RunThreadInner();
}

void *UnixSocketDeletionWatcher::RunThreadInner() {
  char buf[sizeof(struct inotify_event) + NAME_MAX + 1]
     __attribute__ ((aligned(__alignof__(struct inotify_event))));
  fd_set fdset;
  struct stat st;
  for(;;) {
    if (lstat(path_, &st) == -1 && errno == ENOENT) {
      RINFO("Unix socket %s deleted.", path_);
      *flag_to_set_ = true;
      kill(pid_, SIGALRM);
      break;
    }
    FD_ZERO(&fdset);
    FD_SET(inotify_fd_, &fdset);
    FD_SET(pipes_[0], &fdset);
    int n = select(std::max(inotify_fd_, pipes_[0]) + 1, &fdset, NULL, NULL, NULL);
    if (n == -1) {
      if (errno == EINTR)
        continue;
      perror("select");
      break;
    }
    if (FD_ISSET(inotify_fd_, &fdset)) {
      ssize_t len = read(inotify_fd_, buf, sizeof(buf));
      if (len == -1) {
        perror("read");
        break;
      }
    }
    if (FD_ISSET(pipes_[0], &fdset))
      break;
  }
  return NULL;
}

#else  // !defined(OS_LINUX)

bool UnixSocketDeletionWatcher::Poll(const char *path) {
  struct stat st;
  return lstat(path, &st) == -1 && errno == ENOENT;
}

#endif // !defined(OS_LINUX)

UnixDomainSocketListenerBsd::UnixDomainSocketListenerBsd(NetworkBsd *network, WireguardProcessor *processor)
    : BaseSocketBsd(network),
      processor_(processor) {
  memset(&un_addr_, 0, sizeof(un_addr_));
}

UnixDomainSocketListenerBsd::~UnixDomainSocketListenerBsd() {
  if (un_addr_.sun_path[0])
    unlink(un_addr_.sun_path);
}

bool UnixDomainSocketListenerBsd::Initialize(const char *devname) {
  if (!HasFreePollSlot())
    return false;
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd == -1) {
    RERROR("Error creating unix domain socket");
    return false;
  }

  fcntl(fd, F_SETFD, FD_CLOEXEC);
  fcntl(fd, F_SETFL, O_NONBLOCK);

  mkdir("/var/run/wireguard", 0755);
  un_addr_.sun_family = AF_UNIX;
  snprintf(un_addr_.sun_path, sizeof(un_addr_.sun_path), "/var/run/wireguard/%s.sock", devname);
  unlink(un_addr_.sun_path);
  if (bind(fd, (struct sockaddr*)&un_addr_, sizeof(un_addr_)) == -1) {
    RERROR("Error binding unix domain socket");
    close(fd);
    return false;
  }
  if (listen(fd, 5) == -1) {
    RERROR("Error listening on unix domain socket");
    close(fd);
    return false;
  }
  InitPollSlot(fd, POLLIN);
  return true;
}

void UnixDomainSocketListenerBsd::HandleEvents(int revents) {
  if (revents & POLLIN) {
    // wait if we can't allocate more pollfd
    if (!HasFreePollSlot()) {
      SetPollFlags(0);
      return;
    }
    int new_fd = accept(fd_, NULL, NULL);
    if (new_fd >= 0) {
      UnixDomainSocketChannelBsd *channel = new UnixDomainSocketChannelBsd(network_, processor_, new_fd);
    } else {
      RERROR("Unix domain socket accept failed");
    }
  }
  if (revents & ~POLLIN) {
    RERROR("Unix domain socket got an error code");
  }
}

void UnixDomainSocketListenerBsd::Periodic() {
  if (un_deletion_watcher_.Poll(un_addr_.sun_path)) {
    RINFO("Unix socket %s deleted.", un_addr_.sun_path);
    network_->PostExit();
  } else {
    // try again
    SetPollFlags(POLLIN);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////

UnixDomainSocketChannelBsd::UnixDomainSocketChannelBsd(NetworkBsd *network, WireguardProcessor *processor, int fd) 
    : BaseSocketBsd(network),
      processor_(processor) {
  assert(HasFreePollSlot());
  InitPollSlot(fd, POLLIN);
}

UnixDomainSocketChannelBsd::~UnixDomainSocketChannelBsd() {
}

static const char *FindMessageEnd(const char *start, size_t size) {
  if (size <= 1)
    return NULL;
  const char *start_end = start + size - 1;
  for (; (start = (const char*)memchr(start, '\n', start_end - start)) != NULL; start++) {
    if (start[1] == '\n')
      return start + 2;
  }
  return NULL;
}

bool UnixDomainSocketChannelBsd::HandleEventsInner(int revents) {
  if (revents & POLLIN) {
    char buf[4096];
    // read as much data as we can until we see \n\n
    ssize_t n = recv(fd_, buf, sizeof(buf), 0);
    if (n <= 0)
      return (n == -1 && errno == EAGAIN);  // premature eof or error
    inbuf_.append(buf, n);
    const char *message_end = FindMessageEnd(&inbuf_[0], inbuf_.size());
    if (message_end) {
      if (message_end != &inbuf_[inbuf_.size()])
        return false;  // trailing data?
      WgConfig::HandleConfigurationProtocolMessage(processor_, std::move(inbuf_), &outbuf_);
      if (!outbuf_.size())
        return false;
      SetPollFlags(POLLOUT);
      revents |= POLLOUT;
    }
  }
  if (revents & POLLOUT) {
    size_t n = send(fd_, outbuf_.data(), outbuf_.size(), 0);
    if (n <= 0)
      return (n == -1 && errno == EAGAIN);  // premature eof or error
    outbuf_.erase(0, n);
    if (!outbuf_.size())
      return false;
  }
  if (revents & ~(POLLIN | POLLOUT)) {
    RERROR("Unix domain socket got an error code");
    return false;
  }
  return true;
}

void UnixDomainSocketChannelBsd::HandleEvents(int revents) {
  if (!HandleEventsInner(revents))
    delete this;
}

//////////////////////////////////////////////////////////////////////////////////////////////

TcpSocketListenerBsd::TcpSocketListenerBsd(NetworkBsd *bsd, WireguardProcessor *processor)
    : BaseSocketBsd(bsd),
      processor_(processor) {

}

TcpSocketListenerBsd::~TcpSocketListenerBsd() {

}

bool TcpSocketListenerBsd::Initialize(int port) {
  if (!HasFreePollSlot())
    return false;

  CloseSocket();

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) { RERROR("Error listen socket"); return false; }
  fcntl(fd, F_SETFD, FD_CLOEXEC);
  fcntl(fd, F_SETFL, O_NONBLOCK);

  int optval = 1;
 // setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  sockaddr_in sin = {0};
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = INADDR_ANY;
  if (bind(fd, (sockaddr*)&sin, sizeof(sin))) {
    RERROR("Error binding socket on port %d", port);
    close(fd);
    return false;
  }
  if (listen(fd, 5)) {
    RERROR("Error listen socket");
    close(fd);
    return false;
  }
  RINFO("Started TCP listening socket on port %d", port);
  InitPollSlot(fd, POLLIN);
  return true;
}

void TcpSocketListenerBsd::HandleEvents(int revents) {
  if (revents & POLLIN) {
    // wait if we can't allocate more pollfd
    if (!HasFreePollSlot()) {
      SetPollFlags(0);
      return;
    }
    IpAddr addr;
    socklen_t len = sizeof(addr);
    int new_fd = accept(fd_, (sockaddr*)&addr, &len);
    if (new_fd >= 0) {
      RINFO("Created new tcp socket");

      TcpSocketBsd *channel = new TcpSocketBsd(network_, processor_, true);
      if (channel)
        channel->InitializeIncoming(new_fd, addr);
      else
        close(new_fd);
    } else {
      RERROR("Unix domain socket accept failed");
    }
  }
}

void TcpSocketListenerBsd::Periodic() {
  SetPollFlags(POLLIN);
}
//////////////////////////////////////////////////////////////////////////////////////////////

void TcpSocketBsd::WriteTcpPacket(NetworkBsd *network, WireguardProcessor *processor, Packet *packet) {
  bool is_handshake = ReadLE32(packet->data) == MESSAGE_HANDSHAKE_INITIATION;

  // Check if we have a tcp connection for the endpoint, otherwise create one.
  for (TcpSocketBsd *tcp = network->tcp_sockets(); tcp; tcp = tcp->next()) {
    // After we send 3 handshakes on a tcp socket in a row, then close and reopen the socket because it seems defunct.
    if (CompareIpAddr(&tcp->endpoint(), &packet->addr) == 0 && tcp->endpoint_protocol() == packet->protocol) {
      if (is_handshake) {
        uint32 now = (uint32)OsGetMilliseconds();
        uint32 secs = (now - tcp->handshake_timestamp_) >> 10;
        tcp->handshake_timestamp_ += secs * 1024;
        int calc = (secs > (uint32)tcp->handshake_attempts_ + 25) ? 0 : tcp->handshake_attempts_ + 25 - secs;
        tcp->handshake_attempts_ = calc;
        if (calc >= 60) {
          RINFO("Making new Tcp socket due to too many handshake failures");
          delete tcp;
          break;
        }
      }
      tcp->WritePacket(packet);
      return;
    }
  }
  // Drop tcp packet that's for an incoming connection, or packets that are
  // not a handshake.
  if ((packet->protocol & kPacketProtocolIncomingConnection) || !is_handshake) {
    FreePacket(packet);
    return;
  }
  // Initialize a new tcp socket and connect to the endpoint
  TcpSocketBsd *tcp = new TcpSocketBsd(network, processor, false);
  if (!tcp || !tcp->InitializeOutgoing(packet->addr)) {
    delete tcp;
    FreePacket(packet);
    return;
  }
  tcp->WritePacket(packet);
}


//////////////////////////////////////////////////////////////////////////////////////////////

TcpSocketBsd::TcpSocketBsd(NetworkBsd *net, WireguardProcessor *processor, bool is_incoming) 
    : BaseSocketBsd(net),
      readable_(false),
      writable_(true),
      endpoint_protocol_(0),
      age(0),
      handshake_attempts_(0),
      handshake_timestamp_(0),
      wqueue_(NULL),
      wqueue_end_(&wqueue_),
      wqueue_packets_(0),
      processor_(processor),
      tcp_packet_handler_(&net->packet_pool_, &processor->dev().packet_obfuscator(), is_incoming) {
  // insert in network's linked list
  next_ = net->tcp_sockets_;
  net->tcp_sockets_ = this;

  network_->EnsureIovAllocated();
}

TcpSocketBsd::~TcpSocketBsd() {
  // Unlink myself from the network's linked list.
  TcpSocketBsd **p = &network_->tcp_sockets_;
  while (*p != this) p = &(*p)->next_;
  *p = next_;

  RINFO("Destroyed tcp socket");
}

void TcpSocketBsd::InitializeIncoming(int fd, const IpAddr &addr) {
  assert(fd_ == -1);
  endpoint_protocol_ = kPacketProtocolTcp | kPacketProtocolIncomingConnection;
  endpoint_ = addr;
  InitPollSlot(fd, POLLIN);
}

bool TcpSocketBsd::InitializeOutgoing(const IpAddr &addr) {
  assert(fd_ == -1);
  if (!HasFreePollSlot() || addr.sin.sin_family == 0)
    return false;

  endpoint_protocol_ = kPacketProtocolTcp;
  endpoint_ = addr;
  writable_ = false;

  int fd = socket(addr.sin.sin_family, SOCK_STREAM, 0);
  if (fd < 0) { perror("socket: outgoing tcp"); return false; }
  fcntl(fd, F_SETFD, FD_CLOEXEC);
  fcntl(fd, F_SETFL, O_NONBLOCK);

  char buf[kSizeOfAddress];
  RINFO("Connecting to tcp://%s:%d...", PrintIpAddr(endpoint_, buf), ReadBE16(&endpoint_.sin.sin_port));

  if (connect(fd, (sockaddr*)&endpoint_.sin,
              endpoint_.sin.sin_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6))) {
    if (errno != EINPROGRESS) {
      perror("connect: outgoing tcp");
      close(fd);
      return false;
    }
  }

  InitPollSlot(fd, POLLOUT | POLLIN);
  return true;
}

void TcpSocketBsd::WritePacket(Packet *packet) {
  assert(fd_ >= 0);


  Packet *old_value = wqueue_;
  *wqueue_end_ = packet;
  wqueue_end_ = &Packet_NEXT(packet);
  packet->queue_next = NULL;
  packet->prepared = false;

  AddToEndLoop();

  // Note: Cannot use bytes here, because the TCP packet
  // headers have not been added yet, and then the
  // accounting doesn't work
  wqueue_packets_++;

  // When enough packets have been queued up, perform the write.
  if (writable_ && wqueue_packets_ >= 16)
    DoWrite();
}

void TcpSocketBsd::HandleEvents(int revents) {
  if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
    RINFO("TcpSocket error");
    CloseSocketAndDestroy();
    return;
  }

  if (revents & POLLOUT) {
    SetPollFlags(POLLIN);
    AddToEndLoop();
    writable_ = true;
  }

  if (revents & POLLIN)
    DoRead();
}

void TcpSocketBsd::DoEndloop() {
  if (writable_ && wqueue_)
    DoWrite();
}

void TcpSocketBsd::DoRead() {
  ssize_t bytes_read = readv(fd_, network_->iov_, NetworkBsd::kMaxIovec);
  ssize_t bytes_read_org = bytes_read;
  if (bytes_read < 0) {
    if (errno != EAGAIN) {
      RERROR("tcp readv says error code: %d", errno);
      CloseSocketAndDestroy();
    }
    return;
  }
  // Go through and read the packet structures that are ready and queue them up
  NetworkBsd *net = network_;
  for (size_t j = 0; bytes_read; j++) {
    size_t m = std::min<size_t>(bytes_read, net->iov_[j].iov_len);
    Packet *p = net->iov_packets_[j];
    p->size = (int)m;
    bytes_read -= m;
    tcp_packet_handler_.QueueIncomingPacket(p);
    net->ReallocateIov(j);
  }
  // Parse it all
  while (Packet *p = tcp_packet_handler_.GetNextWireguardPacket()) {
    p->protocol = endpoint_protocol_;
    p->addr = endpoint_;
    processor_->HandleUdpPacket(p, network_->overload_);
  }

  if (tcp_packet_handler_.error() || bytes_read_org == 0)
    CloseSocketAndDestroy();
}

void TcpSocketBsd::DoWrite() {
  enum { kMaxIoWrite = 16 };
  struct iovec vecs[kMaxIoWrite];
  Packet *p = wqueue_;
  size_t nvec = 0;
  for (; p && nvec < kMaxIoWrite; p = Packet_NEXT(p)) {
    if (!p->prepared)
      tcp_packet_handler_.PrepareOutgoingPackets(p);

    if (p->size != 0) {
      vecs[nvec].iov_base = p->data;
      vecs[nvec].iov_len = p->size;
      nvec++;
    }
  }
  if (nvec == 0)
    return;

  ssize_t n = writev(fd_, vecs, nvec);

  if (n < 0) {
    if (errno != EAGAIN) {
      RERROR("tcp writev says error code: %d", errno);
      CloseSocketAndDestroy();
    } else {
      writable_ = false;
      SetPollFlags(POLLIN | POLLOUT);
    }
    return;
  }
  // discard those initial n bytes worth of packets
  p = wqueue_;
  while (n) {
    if (n < p->size) {
      p->data += n, p->size -= n;
      break;
    }
    n -= p->size;
    FreePacket(exch(p, Packet_NEXT(p)));
    wqueue_packets_--;
  }
  if (!(wqueue_ = p))
    wqueue_end_ = &wqueue_;
}

void TcpSocketBsd::CloseSocketAndDestroy() {
  delete this;
}

//////////////////////////////////////////////////////////////////////////////////////////////
NotificationPipeBsd::NotificationPipeBsd(NetworkBsd *network)
    : BaseSocketBsd(network),
      injected_cb_(NULL) {

  if (!HasFreePollSlot())
    tunsafe_die("no free poll slots");
  
#if !defined(OS_MACOSX)
  if (pipe2(pipe_fds_, O_CLOEXEC | O_NONBLOCK))
    tunsafe_die("pipe2 failed");
#else
  if (pipe(pipe_fds_))
    tunsafe_die("pipe failed");
  for (int i = 0; i < 2; i++) {
    fcntl(pipe_fds_[i], F_SETFD, FD_CLOEXEC);
    fcntl(pipe_fds_[i], F_SETFL, O_NONBLOCK);
  }
#endif


  InitPollSlot(pipe_fds_[0], POLLIN);
}

NotificationPipeBsd::~NotificationPipeBsd() {
}

void NotificationPipeBsd::InjectCallback(CallbackFunc *func, void *param) {
  CallbackState *st = new CallbackState;
  st->func = func;
  st->param = param;
  // todo: support multiple writers?
  st->next = injected_cb_.exchange(NULL);
  injected_cb_.exchange(st);
  write(pipe_fds_[1], "", 1);
}

void NotificationPipeBsd::Wakeup() {
  write(pipe_fds_[1], "", 1);
}

void NotificationPipeBsd::HandleEvents(int revents) {
  if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
    RERROR("Error with pipe() polling");
    CloseSocket();
  } else if (revents & POLLIN) {
    char tmp[64];
    read(fd_, tmp, sizeof(tmp));
    if (CallbackState *cb = injected_cb_.exchange(NULL)) {
      do {
        CallbackState *next = cb->next;
        cb->func(cb->param);
        cb = next;
      } while (cb);
    }
  }

}
