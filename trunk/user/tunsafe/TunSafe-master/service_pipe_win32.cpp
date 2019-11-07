// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "stdafx.h"
#include "service_pipe_win32.h"
#include "util.h"
#include "service_win32_constants.h"

///////////////////////////////////////////////////////////////////////////////////////
// PipeManager
///////////////////////////////////////////////////////////////////////////////////////

PipeManager::PipeManager(const char *pipe_name, bool is_server_pipe, Delegate *delegate) {
  pipe_name_ = _strdup(pipe_name);
  is_server_pipe_ = is_server_pipe;
  for (size_t i = 0; i < kMaxConnections * 2 + 1; i++)
    events_[i] = CreateEvent(NULL, i != 0, FALSE, NULL); // For Exit
  delegate_ = delegate;
  thread_ = NULL;
  exit_thread_ = false;
  thread_id_ = 0;
  for (size_t i = 0; i != kMaxConnections; i++)
    connections_[i].Configure(this, (int)i);
  connections_[0].state_ = PipeConnection::kStateStarting;
}

PipeManager::~PipeManager() {
  StopThread();
  for (size_t i = 0; i < kMaxConnections * 2 + 1; i++)
    CloseHandle(events_[i]);
  free(pipe_name_);
}

bool PipeManager::StartThread() {
  assert(thread_ == NULL);
  thread_ = CreateThread(NULL, 0, &StaticThreadMain, this, 0, &thread_id_);
  return thread_ != NULL;
}

void PipeManager::StopThread() {
  if (thread_ != NULL) {
    exit_thread_ = true;
    SetEvent(events_[0]);
    WaitForSingleObject(thread_, INFINITE);
    CloseHandle(thread_);
    thread_ = NULL;
  }
}

bool PipeManager::VerifyThread() {
  return thread_id_ == GetCurrentThreadId();
}

void PipeManager::TryStartNewListener() {
  assert(VerifyThread());
  assert(is_server_pipe_);
  // Check if any thread is in the listener state, if not, start
  PipeConnection *found_conn = NULL;
  for (size_t i = 0; i < kMaxConnections; i++) {
    PipeConnection *conn = &connections_[i];
    if (conn->connection_established_)
      continue;
    if (conn->state_ == PipeConnection::kStateWaitConnect)
      return;
    if (conn->state_ == PipeConnection::kStateNone && found_conn == NULL)
      found_conn = conn;
  }
  if (found_conn) {
    found_conn->state_ = PipeConnection::kStateStarting;
    found_conn->AdvanceStateMachine();
  }
}

DWORD WINAPI PipeManager::StaticThreadMain(void *x) {
  return ((PipeManager*)x)->ThreadMain();
}

DWORD PipeManager::ThreadMain() {
  assert(VerifyThread());

  for (size_t i = 0; i < kMaxConnections; i++)
    connections_[i].AdvanceStateMachine();

  for (;;) {
    DWORD rv = WaitForMultipleObjects(1 + kMaxConnections * 2, events_, FALSE, INFINITE);

    // notify?
    if (rv == WAIT_OBJECT_0) {
      if (exit_thread_)
        break;

      delegate_->HandleNotify();
      // The notification event is set when there might be new messages to send,
      // so try to send them.
      for (size_t i = 0; i != kMaxConnections; i++)
        connections_[i].TrySendNextQueuedWrite();
    } else if (rv >= WAIT_OBJECT_0 + 1 && rv < WAIT_OBJECT_0 + 1 + kMaxConnections * 2) {
      PipeConnection *conn = &connections_[(rv - 1) >> 1];
      if (rv & 1) {
        // read finished
        conn->AdvanceStateMachine();
      } else {
        // is the write event
        conn->HandleWriteComplete();
      }
    } else {
      assert(0);
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// PipeConnection
///////////////////////////////////////////////////////////////////////////////////////

static void ClearPipeOverlapped(OVERLAPPED *ov) {
  ov->Internal = 0;
  ov->InternalHigh = 0;
  ov->Offset = 0;
  ov->OffsetHigh = 0;
}

PipeConnection::PipeConnection() {
  pipe_ = INVALID_HANDLE_VALUE;
  packets_ = NULL;
  packets_end_ = &packets_;
  write_overlapped_active_ = false;
  connection_established_ = false;
  state_ = kStateNone;
  tmp_packet_buf_ = NULL;
  tmp_packet_size_ = 0;
  manager_ = NULL;
  delegate_ = NULL;
}

PipeConnection::~PipeConnection() {
}

void PipeConnection::Configure(PipeManager *manager, int slot) {
  manager_ = manager;
  read_overlapped_.hEvent = manager->events_[1 + slot * 2];
  write_overlapped_.hEvent = manager->events_[1 + slot * 2 + 1];
}

int PipeConnection::InitializeServerPipeAndConnect() {
  int BUFSIZE = 8192;
  SECURITY_ATTRIBUTES  saPipeSecurity = {0};
  uint8 buf[SECURITY_DESCRIPTOR_MIN_LENGTH];
  PSECURITY_DESCRIPTOR pPipeSD = (PSECURITY_DESCRIPTOR)buf;

  if (!InitializeSecurityDescriptor(pPipeSD, SECURITY_DESCRIPTOR_REVISION))
    return -1;

  // set NULL DACL on the SD
  if (!SetSecurityDescriptorDacl(pPipeSD, TRUE, (PACL)NULL, FALSE))
    return -1;

  // now set up the security attributes
  saPipeSecurity.nLength = sizeof(SECURITY_ATTRIBUTES);
  saPipeSecurity.bInheritHandle = TRUE;
  saPipeSecurity.lpSecurityDescriptor = pPipeSD;

  pipe_ = CreateNamedPipe(manager_->pipe_name_,
                          PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                          PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_REJECT_REMOTE_CLIENTS | PIPE_WAIT,
                          PIPE_UNLIMITED_INSTANCES,
                          BUFSIZE, BUFSIZE, 0, &saPipeSecurity);
  if (pipe_ == INVALID_HANDLE_VALUE)
    return -1;

  ClearPipeOverlapped(&read_overlapped_);
  // It seems like ConnectNamedPipe never sets the event object if it completes
  // right away.
  if (!ConnectNamedPipe(pipe_, &read_overlapped_)) {
    DWORD rv = GetLastError();
    return (rv == ERROR_IO_PENDING) ? 0 : (rv == ERROR_PIPE_CONNECTED) ? 1 : -1;
  } else {
    return 1;
  }
}

bool PipeConnection::InitializeClientPipe() {
  assert(pipe_ == INVALID_HANDLE_VALUE);
  pipe_ = CreateFile(manager_->pipe_name_, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                     OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
  return (pipe_ != INVALID_HANDLE_VALUE);
}

void PipeConnection::ClosePipe() {
  if (pipe_ != INVALID_HANDLE_VALUE) {
    CancelIo(pipe_);
    CloseHandle(pipe_);
    pipe_ = INVALID_HANDLE_VALUE;
  }
  connection_established_ = false;
  write_overlapped_active_ = false;
  state_ = kStateNone;

  free(tmp_packet_buf_);
  tmp_packet_buf_ = NULL;
  tmp_packet_size_ = 0;

  ResetEvent(read_overlapped_.hEvent);
  ResetEvent(write_overlapped_.hEvent);

  packets_mutex_.Acquire();
  OutgoingPacket *packets = packets_;
  packets_ = NULL;
  packets_end_ = &packets_;
  packets_mutex_.Release();
  while (packets) {
    OutgoingPacket *p = packets;
    packets = p->next;
    free(p);
  }
}

void PipeConnection::HandleWriteComplete() {
  assert(write_overlapped_active_);

  write_overlapped_active_ = false;

  // Remove the packet from the front of the queue, now that it was sent.
  packets_mutex_.Acquire();
  OutgoingPacket *p = packets_;
  if ((packets_ = p->next) == NULL)
    packets_end_ = &packets_;
  packets_mutex_.Release();
  free(p);
  
  if (packets_ == NULL && state_ == kStateWaitTimeout)
    AdvanceStateMachine();
  else
    TrySendNextQueuedWrite();
}

bool PipeConnection::WritePacket(int type, const uint8 *data, size_t data_size) {
  OutgoingPacket *packet = (OutgoingPacket *)malloc(offsetof(OutgoingPacket, data[data_size + 1]));
  if (packet) {
    packet->size = (uint32)(data_size + 1);
    packet->data[0] = type;
    memcpy(packet->data + 1, data, data_size);
    packet->next = NULL;

    packets_mutex_.Acquire();
    OutgoingPacket *was_empty = packets_;
    // login messages are always queued up front
    if (type == TS_SERVICE_REQ_LOGIN) {
      packet->next = packets_;
      if (packet->next == NULL)
        packets_end_ = &packet->next;
      packets_ = packet;
    } else {
      *packets_end_ = packet;
      packets_end_ = &packet->next;
    }
    packets_mutex_.Release();

    if (was_empty == NULL) {
      // Only allow the pipe thread to invoke the send
      if (GetCurrentThreadId() == manager_->thread_id_) {
        TrySendNextQueuedWrite();
      } else {
        SetEvent(manager_->notify_handle());
      }
    }
  }
  return true;
}

bool PipeConnection::VerifyThread() {
  return manager_->VerifyThread();
}

void PipeConnection::TrySendNextQueuedWrite() {
  assert(manager_->VerifyThread());
  if (!write_overlapped_active_) {
    OutgoingPacket *p = packets_;
    if (p && connection_established_) {
      ClearPipeOverlapped(&write_overlapped_);
      if (WriteFile(pipe_, &p->size, p->size + 4, NULL, &write_overlapped_) || GetLastError() == ERROR_IO_PENDING)
        write_overlapped_active_ = true;
    } else {
      ResetEvent(write_overlapped_.hEvent);
    }
  }
}

#define TS_WAIT_BEGIN(t) switch(state_) { case t:
#define TS_WAIT_POINT(t) state_ = (t); return; case t:
#define TS_WAIT_END() }

void PipeConnection::AdvanceStateMachine() {
  DWORD rv;
  int srv;

  TS_WAIT_BEGIN(kStateStarting)
  // Create a named pipe and wait for connections from the UI process
  if (manager_->is_server_pipe_) {
    srv = InitializeServerPipeAndConnect();
    if (srv < 0) {
      if (!manager_->exit_thread_)
        ExitProcess(1);
      ClosePipe();
      return;
    }
    if (srv == 0) {
      TS_WAIT_POINT(kStateWaitConnect);
    }
  } else {
    if (!InitializeClientPipe()) {
      RINFO("Unable to connect to the TunSafe Service. Please make sure it's running.");
      ClosePipe();
      return;
    }
  }
  connection_established_ = true;
  delegate_ = manager_->delegate_->HandleNewConnection(this);
  TrySendNextQueuedWrite();

  for (;;) {
    // Read the packet length
    read_pos_ = 0;
    do {
      ClearPipeOverlapped(&read_overlapped_);
      if (!ReadFile(pipe_, (uint8*)&packet_size_ + read_pos_, 4 - read_pos_, NULL, &read_overlapped_)) {
        if ((rv = GetLastError()) != ERROR_IO_PENDING)
          goto fail;
        TS_WAIT_POINT(kStateWaitReadLength);
      }
      if ((uint32)read_overlapped_.InternalHigh == 0)
        goto fail;
      read_pos_ += (uint32)read_overlapped_.InternalHigh;
    } while (read_pos_ != 4);
    assert(packet_size_ != 0 && packet_size_ < 0x1000000);
    if (packet_size_ == 0 || packet_size_ >= 0x1000000)
      break;
    free(tmp_packet_buf_);
    tmp_packet_buf_ = (uint8*)malloc(packet_size_);
    if (!tmp_packet_buf_)
      break;
    // Read the packet payload
    read_pos_ = 0;
    do {
      ClearPipeOverlapped(&read_overlapped_);
      if (!ReadFile(pipe_, tmp_packet_buf_ + read_pos_, packet_size_ - read_pos_, NULL, &read_overlapped_)) {
        if ((rv = GetLastError()) != ERROR_IO_PENDING)
          goto fail;
        TS_WAIT_POINT(kStateWaitReadPayload);
      }
      if ((uint32)read_overlapped_.InternalHigh == 0)
        goto fail;
      read_pos_ += (uint32)read_overlapped_.InternalHigh;
    } while (read_pos_ != packet_size_);
    if (!delegate_->HandleMessage(tmp_packet_buf_[0], tmp_packet_buf_ + 1, packet_size_ - 1)) {
      ResetEvent(read_overlapped_.hEvent);
      if (packets_ != NULL) {
        TS_WAIT_POINT(kStateWaitTimeout);
      }
      break;
    }
  }
fail:
  ClosePipe();
  if (!manager_->exit_thread_) {
    delegate_->HandleDisconnect();
    if (manager_->is_server_pipe_)
      manager_->TryStartNewListener();
  }
  TS_WAIT_END()

  
}

