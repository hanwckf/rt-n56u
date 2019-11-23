// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#ifndef TUNSAFE_NETWORK_BSD_H_
#define TUNSAFE_NETWORK_BSD_H_

#include <poll.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <string>
#include "network_common.h"

class BaseSocketBsd;
class TcpSocketBsd;
class WireguardProcessor;
class Packet;

class NetworkBsd {
  friend class BaseSocketBsd;
  friend class TcpSocketBsd;
  friend class UdpSocketBsd;
  friend class TunSocketBsd;
public:
  enum {
#if defined(OS_ANDROID)
    WithSigalarmSupport = 0,
#else
    WithSigalarmSupport = 1
#endif
  };

  class NetworkBsdDelegate {
  public:
    virtual void OnSecondLoop(uint64 now) {}
    virtual void RunAllMainThreadScheduled() {}
  };

  explicit NetworkBsd(NetworkBsdDelegate *delegate, int max_sockets);
  ~NetworkBsd();

  void RunLoop(const sigset_t *sigmask);
  void PostExit() { exit_ = true; }

  bool *exit_flag() { return &exit_; }
  bool *sigalarm_flag() { return &sigalarm_flag_; }

  TcpSocketBsd *tcp_sockets() { return tcp_sockets_; }
  bool overload() { return overload_; }
private:
  void RemoveFromRoundRobin(int slot);

  void ReallocateIov(size_t i);
  void EnsureIovAllocated();

  Packet *read_packet_;
  bool exit_;
  bool overload_;
  bool sigalarm_flag_;

  enum {
    // This controls the max # of sockets we can support
    kMaxIovec = 16,
  };
  int num_sock_;
  int num_roundrobin_;
  int num_endloop_;
  int max_sockets_;

  SimplePacketPool packet_pool_;
  NetworkBsdDelegate *delegate_;
  
  struct pollfd *pollfd_;
  BaseSocketBsd **sockets_;
  BaseSocketBsd **roundrobin_;
  BaseSocketBsd **endloop_;

  // Linked list of all tcp sockets
  TcpSocketBsd *tcp_sockets_;

  struct iovec iov_[kMaxIovec];
  Packet *iov_packets_[kMaxIovec];

};

class BaseSocketBsd {
  friend class NetworkBsd;
public:
  BaseSocketBsd(NetworkBsd *network) : pollfd_slot_(-1), roundrobin_slot_(-1), endloop_slot_(-1), fd_(-1), network_(network) {}
  virtual ~BaseSocketBsd();

  virtual void HandleEvents(int revents) = 0;

  // Return |false| to remove socket from roundrobin list.
  virtual bool DoRoundRobin() { return false; }
  virtual void DoEndloop() {}
  virtual void Periodic() {}

  // Make sure this socket gets called during each round robin step.
  void AddToRoundRobin();

  // Make sure this sockets get called at the end of the loop
  void AddToEndLoop();

  int GetFd() { return fd_; }
  int StealFd();

protected:
  void SetPollFlags(int events) {
    network_->pollfd_[pollfd_slot_].events = events;
  }
  void InitPollSlot(int fd, int events);
  bool HasFreePollSlot() { return network_->num_sock_ != network_->max_sockets_; }
  void CloseSocket();

  NetworkBsd *network_;
  int pollfd_slot_;
  int roundrobin_slot_;
  int endloop_slot_;
  int fd_;
};

class TunSocketBsd : public BaseSocketBsd {
public:
  explicit TunSocketBsd(NetworkBsd *network, WireguardProcessor *processor);
  virtual ~TunSocketBsd();

  bool Initialize(int fd);

  virtual void HandleEvents(int revents) override;
  virtual bool DoRoundRobin() override;

  void WritePacket(Packet *packet);

  bool tun_interface_gone() const { return tun_interface_gone_; }

private:
  bool DoRead();
  bool DoWrite();

  bool tun_readable_, tun_writable_;
  bool tun_interface_gone_;
  Packet *tun_queue_, **tun_queue_end_;
  WireguardProcessor *processor_;
};

class UdpSocketBsd : public BaseSocketBsd {
public:
  explicit UdpSocketBsd(NetworkBsd *network, WireguardProcessor *processor);
  virtual ~UdpSocketBsd();

  bool Initialize(int listen_port);

  virtual void HandleEvents(int revents) override;
  virtual bool DoRoundRobin() override;

  bool DoRead();
  bool DoWrite();

  void WritePacket(Packet *packet);
  
private:
  bool udp_readable_, udp_writable_;
  Packet *udp_queue_, **udp_queue_end_;
  WireguardProcessor *processor_;
};

#if defined(OS_LINUX)
// Keeps track of when the unix socket gets deleted
class UnixSocketDeletionWatcher {
public:
  UnixSocketDeletionWatcher();
  ~UnixSocketDeletionWatcher();
  bool Start(const char *path, bool *flag_to_set);
  void Stop();
  bool Poll(const char *path) { return false; }
 
private:
  static void *RunThread(void *arg);
  void *RunThreadInner();
  const char *path_;
  int inotify_fd_;
  int pid_;
  int pipes_[2];
  pthread_t thread_;
  bool *flag_to_set_;
};
#else  // !defined(OS_LINUX)
// all other platforms that lack inotify
class UnixSocketDeletionWatcher {
public:
  UnixSocketDeletionWatcher() {}
  ~UnixSocketDeletionWatcher() {}
  bool Start(const char *path, bool *flag_to_set) { return true; }
  void Stop() {}
  bool Poll(const char *path);
};
#endif  // !defined(OS_LINUX)

class UnixDomainSocketListenerBsd : public BaseSocketBsd {
public:
  explicit UnixDomainSocketListenerBsd(NetworkBsd *network, WireguardProcessor *processor);
  virtual ~UnixDomainSocketListenerBsd();

  bool Initialize(const char *devname);

  bool Start(bool *exit_flag) {
    return un_deletion_watcher_.Start(un_addr_.sun_path, exit_flag);
  }
  void Stop() { un_deletion_watcher_.Stop(); }

  virtual void HandleEvents(int revents) override;
  virtual void Periodic() override;
private:
  struct sockaddr_un un_addr_;
  WireguardProcessor *processor_;
  UnixSocketDeletionWatcher un_deletion_watcher_;
};

class UnixDomainSocketChannelBsd : public BaseSocketBsd {
public:
  explicit UnixDomainSocketChannelBsd(NetworkBsd *network, WireguardProcessor *processor, int fd);
  virtual ~UnixDomainSocketChannelBsd();

  virtual void HandleEvents(int revents) override;

private:
  bool HandleEventsInner(int revents);
  WireguardProcessor *processor_;
  std::string inbuf_, outbuf_;
};

class TcpSocketListenerBsd : public BaseSocketBsd {
public:
  explicit TcpSocketListenerBsd(NetworkBsd *bsd, WireguardProcessor *processor);
  virtual ~TcpSocketListenerBsd();

  bool Initialize(int port);

  virtual void HandleEvents(int revents) override;
  virtual void Periodic() override;

private:
  WireguardProcessor *processor_;
};

class TcpSocketBsd : public BaseSocketBsd {
public:
  explicit TcpSocketBsd(NetworkBsd *bsd, WireguardProcessor *processor, bool is_incoming);
  virtual ~TcpSocketBsd();

  void InitializeIncoming(int fd, const IpAddr &addr);
  bool InitializeOutgoing(const IpAddr &addr);

  void WritePacket(Packet *packet);

  virtual void HandleEvents(int revents) override;
  virtual void DoEndloop() override;

  TcpSocketBsd *next() { return next_; }
  uint8 endpoint_protocol() { return endpoint_protocol_; }
  const IpAddr &endpoint() { return endpoint_; }

  static void WriteTcpPacket(NetworkBsd *network, WireguardProcessor *processor, Packet *packet);

public:
  uint8 age;
private:
  void DoRead();
  void DoWrite();
  void CloseSocketAndDestroy();

  bool readable_, writable_;
  bool got_eof_;
  uint8 endpoint_protocol_;
  bool want_connect_;
  uint8 handshake_attempts_;
  uint32 handshake_timestamp_;
  
  uint wqueue_packets_;
  Packet *wqueue_, **wqueue_end_;
  TcpSocketBsd *next_;
  WireguardProcessor *processor_;
  TcpPacketHandler tcp_packet_handler_;
  IpAddr endpoint_;
};

class NotificationPipeBsd : public BaseSocketBsd {
public:
  NotificationPipeBsd(NetworkBsd *network);
  ~NotificationPipeBsd();

  typedef void CallbackFunc(void *x);
  void InjectCallback(CallbackFunc *func, void *param);
  void Wakeup();

  virtual void HandleEvents(int revents) override;

private:
  struct CallbackState {
    CallbackFunc *func;
    void *param;
    CallbackState *next;
  };
  int pipe_fds_[2];
  std::atomic<CallbackState*> injected_cb_;
};


#endif  // TUNSAFE_NETWORK_BSD_H_