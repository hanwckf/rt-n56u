// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once
#include "tunsafe_threading.h"
#include "network_win32_api.h"

class PipeManager;

// Once a pipe connects, this object is used to facilitate the connection 
class PipeConnection {
  friend class PipeManager;
public:
  class Delegate {
  public:
    virtual bool HandleMessage(int type, uint8 *data, size_t size) = 0;
    virtual void HandleDisconnect() = 0;
  };

  PipeConnection();
  ~PipeConnection();

  void Configure(PipeManager *manager, int slot);
  bool WritePacket(int type, const uint8 *data, size_t data_size);
  HANDLE pipe_handle() { return pipe_; }
  bool is_connected() { return connection_established_; }
  bool VerifyThread();

private:
  // -1 = fail, 0 = wait, 1 = conn
  int InitializeServerPipeAndConnect();
  bool InitializeClientPipe();
  void AdvanceStateMachine();
  void ClosePipe();
  void TrySendNextQueuedWrite();
  void HandleWriteComplete();
  
  Delegate *delegate_;
  PipeManager *manager_;

  HANDLE pipe_;
  bool write_overlapped_active_;
  bool connection_established_;

  enum State {
    kStateNone,
    kStateStarting,
    kStateWaitConnect,
    kStateWaitReadLength,
    kStateWaitReadPayload,
    kStateWaitTimeout,
  };

  uint8 state_;

  uint32 packet_size_;
  
  struct OutgoingPacket {
    OutgoingPacket *next;
    uint32 size;
    uint8 data[0];
  };
  OutgoingPacket *packets_, **packets_end_;
  uint8 *tmp_packet_buf_;
  DWORD tmp_packet_size_;
  DWORD read_pos_;
  OVERLAPPED write_overlapped_, read_overlapped_;
  Mutex packets_mutex_;
};

// This class supports multiple PipeConnections and calls HandleNewConnection
// when a new pipe connection is established.
class PipeManager {
  friend class PipeConnection;
public:
  class Delegate {
  public:
    // Called when a new connection is established
    virtual PipeConnection::Delegate *HandleNewConnection(PipeConnection *handler) = 0;

    // Called when a notification event was pushed
    virtual void HandleNotify() = 0;
  };

  PipeManager(const char *pipe_name, bool is_server_pipe, Delegate *delegate);
  ~PipeManager();

  bool StartThread();
  void StopThread();
  bool VerifyThread();

  HANDLE notify_handle() { return events_[0]; }
  PipeConnection *GetClientConnection() { return &connections_[0]; }

  void TryStartNewListener();

private:
  DWORD ThreadMain();
  static DWORD WINAPI StaticThreadMain(void *x);

  Delegate *delegate_;
  HANDLE thread_;
  char *pipe_name_;
  DWORD thread_id_;
  bool is_server_pipe_;
  bool exit_thread_;

  enum { kMaxConnections = 2 };
  HANDLE events_[1 + kMaxConnections * 2];
  PipeConnection connections_[kMaxConnections];
};

