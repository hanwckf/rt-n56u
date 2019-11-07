// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
// Note: This is an experimental implementation that doesn't work, there's no way
// for the alarm signal to interrupt the tunsafe main thread.
#include "network_bsd_common.h"
#include "tunsafe_endian.h"
#include "tunsafe_config.h"
#include "tunsafe_threading.h"
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
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>

static Packet *freelist;

void FreePacket(Packet *packet) {
  free(packet);
//  packet->next = freelist;
//  freelist = packet;
}

Packet *AllocPacket() {
  Packet *p = NULL;// freelist;
  if (p) {
    freelist = p->next;
  } else {
    p = (Packet*)malloc(kPacketAllocSize);  
    if (p == NULL) {
      RERROR("Allocation failure");
      abort();
    }
  }
  p->data = p->data_buf + Packet::HEADROOM_BEFORE;
  p->size = 0;
  return p;
}

void FreePackets() {
  Packet *p;
  while ( (p = freelist ) != NULL) {
    freelist = p->next;
    free(p);
  }
}

class WorkerLoop {
public:
  WorkerLoop();
  ~WorkerLoop();

  bool Initialize(WireguardProcessor *processor);

  void *ThreadMain();
  void StartThread();

  void StopThread();

  void NotifyStop();

  enum {
    TARGET_UDP, TARGET_TUN
  };

  void HandleUdpPacket(Packet *packet) {
    HandlePacket(packet, TARGET_UDP);
  }
  void HandleTunPacket(Packet *packet) {
    HandlePacket(packet, TARGET_TUN);
  }

  void HandleSigAlrm() {
    got_sig_alarm_ = true;
  }

private:
  static void *ThreadMainStatic(void *x);
  void HandlePacket(Packet *packet, int target);

  WireguardProcessor *processor_;
  pthread_t tid_;
  Packet *queue_, **queue_end_;
  bool shutting_down_;
  bool got_sig_alarm_;

  Mutex lock_;
  pthread_cond_t cond_;
};

// Handles the threads that read/write to the udp socket.
class UdpLoop {
public:
  UdpLoop();
  ~UdpLoop();

  bool Initialize(int listen_port, WorkerLoop *worker);
  void Start();
  void Stop();

  void WriteUdpPacket(Packet *packet);
private:
  static void *ReaderMainStatic(void *x);
  static void *WriterMainStatic(void *x);
  void *ReaderMain();
  void *WriterMain();
  
  int fd_;
  WorkerLoop *worker_;
  pthread_t read_tid_, write_tid_;

  Packet *queue_, **queue_end_;

  bool shutting_down_;

  Mutex lock_;
  pthread_cond_t cond_;
};

// Handles the threads that read/write to the tun socket.
class TunLoop {
public:
  TunLoop();
  ~TunLoop();

  bool Initialize(char devname[16], WorkerLoop *worker);
  void Start();
  void Stop();

  void WriteTunPacket(Packet *packet);
private:
  static void *ReaderMainStatic(void *x);
  static void *WriterMainStatic(void *x);
  void *ReaderMain();
  void *WriterMain();

  int fd_;
  bool shutting_down_;

  WorkerLoop *worker_;
  pthread_t read_tid_, write_tid_;
  Packet *queue_, **queue_end_;
  Mutex lock_;
  pthread_cond_t cond_;
};

WorkerLoop::WorkerLoop() {
  queue_end_ = &queue_;
  queue_ = NULL;
  tid_ = 0;
  shutting_down_ = false;
  got_sig_alarm_ = false;
  processor_ = NULL;
  if (pthread_cond_init(&cond_, NULL) != 0)
    tunsafe_die("pthread_cond_init failed");
}

WorkerLoop::~WorkerLoop() {
  pthread_cond_destroy(&cond_);
}

bool WorkerLoop::Initialize(WireguardProcessor *processor) {
  processor_ = processor;
  return true;
}

void WorkerLoop::StartThread() {
  assert(tid_ == 0);
  if (pthread_create(&tid_, NULL, &ThreadMainStatic, this) != 0)
    tunsafe_die("pthread_create failed");
}

void WorkerLoop::StopThread() {
  lock_.Acquire();
  shutting_down_ = true;
  lock_.Release();

  if (tid_) {
    void *x;
    pthread_join(tid_, &x);
    tid_ = 0;
  }
}


// This is called from signal handler so cannot block etc.
void WorkerLoop::NotifyStop() {
  shutting_down_ = true;
}

void WorkerLoop::HandlePacket(Packet *packet, int target) {
//  RINFO("WorkerLoop::HandlePacket");
  packet->post_target = target;
  lock_.Acquire();
  Packet *old_queue = queue_;
  *queue_end_ = packet;
  queue_end_ = &packet->next;
  packet->next = NULL;
  if (old_queue == NULL) {
    lock_.Release();
    pthread_cond_signal(&cond_);
  } else {
    lock_.Release();
  }
}

void *WorkerLoop::ThreadMainStatic(void *x) {
  return ((WorkerLoop*)x)->ThreadMain();
}

void *WorkerLoop::ThreadMain() {
  Packet *packet_queue;

  lock_.Acquire();
  for (;;) {
    // Grab the whole list
    for (;;) {
      while (got_sig_alarm_) {
        got_sig_alarm_ = false;
        lock_.Release();
        processor_->SecondLoop();
        processor_->RunAllMainThreadScheduled();
        lock_.Acquire();
      }
      if (shutting_down_ || queue_ != NULL)
        break;
      pthread_cond_wait(&cond_, lock_.impl());
    }
    if (shutting_down_)
      break;
    packet_queue = queue_;
    queue_ = NULL;
    queue_end_ = &queue_;
    
    lock_.Release();
    // And send all items in the list
    while (packet_queue != NULL) {
      Packet *next = packet_queue->next;
      if (packet_queue->post_target == TARGET_TUN) {
        processor_->HandleTunPacket(packet_queue);
      } else {
        processor_->HandleUdpPacket(packet_queue, false);
      }
      packet_queue = next;
    }
    processor_->RunAllMainThreadScheduled();
    lock_.Acquire();
  }
  lock_.Release();
  return NULL;
}



UdpLoop::UdpLoop() {
  fd_ = -1;
  read_tid_ = 0;
  write_tid_ = 0;
  shutting_down_ = false;
  worker_ = NULL;
  queue_ = NULL;
  queue_end_ = &queue_;
  if (pthread_cond_init(&cond_, NULL) != 0)
    tunsafe_die("pthread_cond_init failed");
}

UdpLoop::~UdpLoop() {
  if (fd_ != -1)
    close(fd_);
  pthread_cond_destroy(&cond_);
}

bool UdpLoop::Initialize(int listen_port, WorkerLoop *worker) {
  int fd = open_udp(listen_port);
  if (fd < 0) { RERROR("Error opening udp"); return false; }
  fcntl(fd, F_SETFD, FD_CLOEXEC);
  fd_ = fd;
  worker_ = worker;
  return true;
}

void UdpLoop::Start() {
  if (pthread_create(&read_tid_, NULL, &ReaderMainStatic, this) != 0)
    tunsafe_die("pthread_create failed");
  if (pthread_create(&write_tid_, NULL, &WriterMainStatic, this) != 0)
    tunsafe_die("pthread_create failed");
}

void UdpLoop::Stop() {
  void *x;

  lock_.Acquire();
  shutting_down_ = true;
  lock_.Release();
  pthread_cond_signal(&cond_);

  pthread_kill(read_tid_, SIGUSR1);
  pthread_kill(write_tid_, SIGUSR1);
  
  pthread_join(read_tid_, &x);
  pthread_join(write_tid_, &x);

  read_tid_ = 0;
  write_tid_ = 0;
}

void *UdpLoop::ReaderMainStatic(void *x) {
  SetThreadName("tunsafe-ur");
  return ((UdpLoop*)x)->ReaderMain();
}

void *UdpLoop::WriterMainStatic(void *x) {
  SetThreadName("tunsafe-uw");
  return ((UdpLoop*)x)->WriterMain();
}

void *UdpLoop::ReaderMain() {
  Packet *packet;
  socklen_t sin_len;
  int r;

  while (!shutting_down_) {
    packet = AllocPacket();
    sin_len = sizeof(packet->addr.sin);
    r = recvfrom(fd_, packet->data, kPacketCapacity, 0, (sockaddr*)&packet->addr.sin, &sin_len);
    if (r < 0) {
      FreePacket(packet);
      if (shutting_down_)
        break;

      RERROR("ReadMain failed %d", errno);

    } else {
      packet->size = r;
      worker_->HandleUdpPacket(packet);
    }
  }
  return NULL;
}

void *UdpLoop::WriterMain() {
  Packet *queue;

  lock_.Acquire();
  for (;;) {
    // Grab the whole list
    while (!shutting_down_ && queue_ == NULL)
      pthread_cond_wait(&cond_, lock_.impl());
    if (shutting_down_)
      break;
    queue = queue_;
    queue_ = NULL;
    queue_end_ = &queue_;
    lock_.Release();
    // And send all items in the list
    while (queue != NULL) {
      int r = sendto(fd_, queue->data, queue->size, 0,
          (sockaddr*)&queue->addr.sin, sizeof(queue->addr.sin));
      if (r != queue->size) {
        if (errno != ENOBUFS)
          RERROR("WriterMain failed: %d", errno);
      } else {
//        RINFO("WRote udp packet!");
      }
      Packet *to_free = queue;
      queue = queue->next;
      FreePacket(to_free);
    }
    lock_.Acquire();
  }
  lock_.Release();
  return NULL;
}

void UdpLoop::WriteUdpPacket(Packet *packet) {
//  RINFO("write udp packet to queue!");
  packet->next = NULL;

  lock_.Acquire();
  Packet *old_queue = queue_;
  *queue_end_ = packet;
  queue_end_ = &packet->next;
  if (old_queue == NULL) {
    lock_.Release();
    pthread_cond_signal(&cond_);
  } else {
    lock_.Release();
  }
}

TunLoop::TunLoop() {
  fd_ = -1;
  shutting_down_ = false;
  worker_ = NULL;
  read_tid_ = 0;
  write_tid_ = 0;
  queue_ = NULL;
  queue_end_ = &queue_;
  if (pthread_cond_init(&cond_, NULL) != 0)
    tunsafe_die("pthread_cond_init failed");
}

TunLoop::~TunLoop() {
  if (fd_ != -1)
    close(fd_);
  pthread_cond_destroy(&cond_);
}

bool TunLoop::Initialize(char devname[16], WorkerLoop *worker) {
  int fd = open_tun(devname, 16);
  if (fd < 0) { RERROR("Error opening tun"); return false; }
  fcntl(fd, F_SETFD, FD_CLOEXEC);
  fd_ = fd;
  worker_ = worker;
  return true;
}

void TunLoop::Start() {
  if (pthread_create(&read_tid_, NULL, &ReaderMainStatic, this) != 0)
    tunsafe_die("pthread_create failed");
  if (pthread_create(&write_tid_, NULL, &WriterMainStatic, this) != 0)
    tunsafe_die("pthread_create failed");
}

void TunLoop::Stop() {
  void *x;

  lock_.Acquire();
  shutting_down_ = true;
  lock_.Release();

  pthread_kill(read_tid_, SIGUSR1);
  pthread_kill(write_tid_, SIGUSR1);
  pthread_join(read_tid_, &x);
  pthread_join(write_tid_, &x);

  read_tid_ = 0;
  write_tid_ = 0;
}

void *TunLoop::ReaderMainStatic(void *x) {
  SetThreadName("tunsafe-tr");
  return ((TunLoop*)x)->ReaderMain();
}

void *TunLoop::WriterMainStatic(void *x) {
  SetThreadName("tunsafe-tw");
  return ((TunLoop*)x)->WriterMain();
}

void *TunLoop::ReaderMain() {
  Packet *packet = AllocPacket();
  while (!shutting_down_) {
    int r = read(fd_, packet->data - TUN_PREFIX_BYTES, kPacketCapacity + TUN_PREFIX_BYTES);
    if (r >= 0) {
      packet->size = r - TUN_PREFIX_BYTES;
      if (r >= TUN_PREFIX_BYTES && (!TUN_PREFIX_BYTES || ReadBE32(packet->data - TUN_PREFIX_BYTES) == AF_INET)) {
        worker_->HandleTunPacket(packet);
        packet = AllocPacket();
      }
    }
  }
  return NULL;
}

void *TunLoop::WriterMain() {
  Packet *queue;

  lock_.Acquire();
  for (;;) {
    // Grab the whole list
    while (!shutting_down_ && queue_ == NULL) {
      pthread_cond_wait(&cond_, lock_.impl());
    }
    if (shutting_down_)
      break;
    queue = queue_;
    queue_ = NULL;
    queue_end_ = &queue_;
    lock_.Release();
    // And send all items in the list
    while (queue != NULL) {
      if (TUN_PREFIX_BYTES)
        WriteBE32(queue->data - TUN_PREFIX_BYTES, AF_INET);
      int r = write(fd_, queue->data - TUN_PREFIX_BYTES, queue->size + TUN_PREFIX_BYTES);
      if (r != queue->size + TUN_PREFIX_BYTES) {
        RERROR("WriterMain failed: %d", errno);
        break;
      }
      Packet *to_free = queue;
      queue = queue->next;
      FreePacket(to_free);
    }
    lock_.Acquire();
  }
  lock_.Release();
  return NULL;
}

void TunLoop::WriteTunPacket(Packet *packet) {
  packet->next = NULL;

  lock_.Acquire();
  Packet *old_queue = queue_;
  *queue_end_ = packet;
  queue_end_ = &packet->next;
  if (old_queue == NULL) {
    lock_.Release();
    pthread_cond_signal(&cond_);
  } else {
    lock_.Release();
  }
}

class TunsafeBackendBsdImpl : public TunsafeBackendBsd {
public:
  TunsafeBackendBsdImpl();
  virtual ~TunsafeBackendBsdImpl();

  // -- from TunInterface
  virtual void WriteTunPacket(Packet *packet) override;

  // -- from UdpInterface
  virtual bool Initialize(int listen_port) override;
  virtual void WriteUdpPacket(Packet *packet) override;

  virtual void HandleSigAlrm() override { worker_.HandleSigAlrm(); }
  virtual void HandleExit() override { worker_.NotifyStop(); }

  virtual bool InitializeTun(char devname[16]) override;  

  virtual void RunLoopInner() override;
private:
  WorkerLoop worker_;
  UdpLoop udp_;
  TunLoop tun_;
};

TunsafeBackendBsdImpl::TunsafeBackendBsdImpl() {
}

TunsafeBackendBsdImpl::~TunsafeBackendBsdImpl() {
}

bool TunsafeBackendBsdImpl::InitializeTun(char devname[16]) {
  return tun_.Initialize(devname, &worker_);
}

void TunsafeBackendBsdImpl::WriteTunPacket(Packet *packet) override {
  tun_.WriteTunPacket(packet);
}

// Called to initialize udp
bool TunsafeBackendBsdImpl::Initialize(int listen_port) override {
  return udp_.Initialize(listen_port, &worker_);
}

void TunsafeBackendBsdImpl::WriteUdpPacket(Packet *packet) override {
  udp_.WriteUdpPacket(packet);
}

void TunsafeBackendBsdImpl::RunLoopInner() {  
  worker_.Initialize(processor_);
  udp_.Start();
  tun_.Start();

  worker_.ThreadMain();

  tun_.Stop();
  udp_.Stop();
}

TunsafeBackendBsd *CreateTunsafeBackendBsd() {
  return new TunsafeBackendBsdImpl;
}
