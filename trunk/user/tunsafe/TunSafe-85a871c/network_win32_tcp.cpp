// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include <stdafx.h>
#include "network_win32_tcp.h"
#include "network_win32.h"
#include <Mswsock.h>
#include <ws2ipdef.h>
#include "util.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////

TcpSocketWin32::TcpSocketWin32(NetworkWin32 *network, PacketProcessor *packet_handler, WgPacketObfuscator *obfuscator, bool is_incoming)
    : packet_processor_(packet_handler), tcp_packet_handler_(&network->packet_pool(), obfuscator, is_incoming) {
  network_ = network;
  reads_active_ = 0;
  writes_active_ = 0;
  handshake_attempts = 0;
  handshake_timestamp_ = 0;
  state_ = STATE_NONE;
  wqueue_ = NULL;
  wqueue_end_ = &wqueue_;
  socket_ = INVALID_SOCKET;
  next_ = NULL;
  // insert in network's linked list
  next_ = network->tcp_socket_;
  network->tcp_socket_ = this;
}

TcpSocketWin32::~TcpSocketWin32() {
  // Unlink myself from the network's linked list.
  TcpSocketWin32 **p = &network_->tcp_socket_;
  while (*p != this) p = &(*p)->next_;
  *p = next_;

  FreePacketList(wqueue_);
  if (socket_ != INVALID_SOCKET)
    closesocket(socket_);
}

void TcpSocketWin32::CloseSocket() {
  if (socket_ != INVALID_SOCKET)
    CancelIo((HANDLE)socket_);
  state_ = STATE_ERROR;
  endpoint_protocol_ = 0;
}

void TcpSocketWin32::WritePacket(Packet *packet) {
  packet->prepared = false;
  packet->queue_next = NULL;
  *wqueue_end_ = packet;
  wqueue_end_ = &Packet_NEXT(packet);
}

void TcpSocketWin32::CancelAllIO() {
  if (socket_ != INVALID_SOCKET)
    CancelIo((HANDLE)socket_);
}

static const GUID WsaConnectExGUID = WSAID_CONNECTEX;

void TcpSocketWin32::DoConnect() {
  LPFN_CONNECTEX ConnectEx;
  assert(socket_ == INVALID_SOCKET);

  socket_ = WSASocket(endpoint_.sin.sin_family, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
  if (socket_ == INVALID_SOCKET) {
    RERROR("socket() failed");
    CloseSocket();
    return;
  }

  if (!CreateIoCompletionPort((HANDLE)socket_, network_->completion_port_handle_, 0, 0)) {
    RERROR("TcpSocketWin32::DoConnect CreateIoCompletionPort failed");
    CloseSocket();
    return;
  }

  int nodelay = 1;
  setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, 1);

  DWORD dwBytes = sizeof(ConnectEx);
  DWORD rc = WSAIoctl(socket_, SIO_GET_EXTENSION_FUNCTION_POINTER, (uint8*)&WsaConnectExGUID, sizeof(WsaConnectExGUID), &ConnectEx, sizeof(ConnectEx), &dwBytes, NULL, NULL);
  assert(rc == 0);

  // ConnectEx requires the socket to be bound
  sockaddr_in sin = {0};
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = 0;
  if (bind(socket_, (sockaddr*)&sin, sizeof(sin))) {
    RERROR("TcpSocketWin32::DoConnect bind failed: %d", WSAGetLastError());
    CloseSocket();
    return;
  }

  char buf[kSizeOfAddress];
  RINFO("Connecting to tcp://%s...", PrintIpAddr(endpoint_, buf));

  state_ = STATE_CONNECTING;
  ClearOverlapped(&connect_overlapped_.overlapped);
  connect_overlapped_.queue_cb = this;
  if (!ConnectEx(socket_, (const sockaddr*)&endpoint_.sin, sizeof(endpoint_.sin), NULL, 0, NULL, &connect_overlapped_.overlapped)) {
    int err = WSAGetLastError();
    if (err != ERROR_IO_PENDING) {
      RERROR("ConnectEx failed: %d", err);
      CloseSocket();
      return;
    }
  }
  reads_active_ = 1;
}

void TcpSocketWin32::DoMoreReads() {
  assert(state_ != STATE_ERROR);
  if (reads_active_ == 0) {
    // Initiate a new read, we always read into 4 buffers.
    Packet *p = network_->packet_pool().AllocPacketFromPool();
    if (!p)
      return;

    ClearOverlapped(&p->overlapped);
    p->userdata = 0;
    p->queue_cb = this;
    DWORD flags = 0;
    WSABUF wsabuf = {(ULONG)kPacketCapacity, (char*)p->data};
    if (WSARecv(socket_, &wsabuf, 1, NULL, &flags, &p->overlapped, NULL) != 0) {
      DWORD err = WSAGetLastError();
      if (err != ERROR_IO_PENDING) {
        RERROR("TcpSocketWin32:WSARecv failed 0x%X", err);
        FreePacket(p);
        return;
      }
    }
    reads_active_ = 1;
  }
}

void TcpSocketWin32::DoMoreWrites() {
  assert(state_ != STATE_ERROR);
  if (writes_active_ == 0) {
    WSABUF wsabuf[kMaxWsaBuf];
    uint32 num_wsabuf = 0;

    Packet *p = wqueue_;
    if (p == NULL)
      return;

    do {
      if (!p->prepared)
        tcp_packet_handler_.PrepareOutgoingPackets(p);

      wsabuf[num_wsabuf].buf = (char*)p->data;
      wsabuf[num_wsabuf].len = (ULONG)p->size;
      packets_in_write_io_[num_wsabuf] = p;
      p = Packet_NEXT(p);
    } while (++num_wsabuf < kMaxWsaBuf && p != NULL);
    if (!(wqueue_ = p))
      wqueue_end_ = &wqueue_;
    num_wsabuf_ = (uint8)num_wsabuf;

    p = packets_in_write_io_[0];
    ClearOverlapped(&p->overlapped);
    p->userdata = 1;
    p->queue_cb = this;

    if (WSASend(socket_, wsabuf, num_wsabuf, NULL, 0, &p->overlapped, NULL) != 0) {
      DWORD err = WSAGetLastError();
      if (err != ERROR_IO_PENDING) {
        RERROR("TcpSocketWin32: WSASend failed 0x%X", err);
        FreePacket(p);
        CloseSocket();
        return;
      }
    }
    writes_active_ = 1;
  }
}

void TcpSocketWin32::DoIO() {
  if (state_ == STATE_CONNECTED) {
    DoMoreReads();
    while (Packet *p = tcp_packet_handler_.GetNextWireguardPacket()) {
      p->protocol = endpoint_protocol_;
      p->addr = endpoint_;
      p->queue_cb = packet_processor_->tcp_queue();
      packet_processor_->ForcePost(p);
    }
    if (tcp_packet_handler_.error()) {
      CloseSocket();
      DoIO();
      return;
    }
    DoMoreWrites();
  } else if (state_ == STATE_WANT_CONNECT) {
    DoConnect();
  } else if (state_ == STATE_ERROR && !HasOutstandingIO()) {
    delete this;
  }
}

bool TcpSocketWin32::HasOutstandingIO() {
  return writes_active_ + reads_active_ != 0;
}

void TcpSocketWin32::OnQueuedItemEvent(QueuedItem *qi, uintptr_t extra) {
  if (qi == &connect_overlapped_) {
    assert(state_ == STATE_CONNECTING);
    reads_active_ = 0;
    if ((DWORD)qi->overlapped.Internal != 0) {
      if (state_ != STATE_ERROR) {
        RERROR("TcpSocketWin32::Connect error 0x%X", (DWORD)qi->overlapped.Internal);
        CloseSocket();
      }
    } else {
      state_ = STATE_CONNECTED;
    }
    return;
  }
  Packet *p = static_cast<Packet*>(qi);
  if (p->userdata == 0) {
    // Read operation complete
    if ((DWORD)p->overlapped.Internal != 0) {
      if (state_ != STATE_ERROR) {
        RERROR("TcpSocketWin32::Read error 0x%X", (DWORD)p->overlapped.Internal);
        CloseSocket();
      }
      network_->packet_pool().FreePacketToPool(p);
      // What to do?
    } else if ((int)p->overlapped.InternalHigh == 0) {
      // Socket closed successfully
      CloseSocket();
      network_->packet_pool().FreePacketToPool(p);
    } else {
      // Queue it up to rqueue
      p->size = (int)p->overlapped.InternalHigh;
      tcp_packet_handler_.QueueIncomingPacket(p);
    }
    reads_active_--;
  } else {
    assert(writes_active_);
    assert(packets_in_write_io_[0] == p);

    if ((DWORD)p->overlapped.Internal != 0) {
      if (state_ != STATE_ERROR) {
        RERROR("TcpSocketWin32::Write error 0x%X", (DWORD)p->overlapped.Internal);
        CloseSocket();
      }
    }
    // free all the packets involved in the write
    for (size_t i = 0; i < num_wsabuf_; i++)
      network_->packet_pool().FreePacketToPool(packets_in_write_io_[i]);
    writes_active_--;
  }
}

void TcpSocketWin32::OnQueuedItemDelete(QueuedItem *qi) {
  if (qi == &connect_overlapped_) {
    reads_active_ = 0;
    return;
  }
  Packet *p = static_cast<Packet*>(qi);
  if (p->userdata == 0) {
    FreePacket(p);
    reads_active_--;
  } else {
    for (size_t i = 0; i < num_wsabuf_; i++)
      network_->packet_pool().FreePacketToPool(packets_in_write_io_[i]);
    writes_active_--;
  }
}

/////////////////////////////////////////////////////////////////////////

TcpSocketQueue::TcpSocketQueue(NetworkWin32 *network, WgPacketObfuscator *obfuscator) {
  network_ = network;
  wqueue_ = NULL;
  wqueue_end_ = &wqueue_;
  queued_item_.queue_cb = this;
  packet_handler_ = NULL;
  obfuscator_ = obfuscator;
}

TcpSocketQueue::~TcpSocketQueue() {
  FreePacketList(wqueue_);
}

void TcpSocketQueue::TransmitPackets(Packet *packet) {
AGAIN:
  while (packet) {
    bool is_handshake = ReadLE32(packet->data) == MESSAGE_HANDSHAKE_INITIATION;

    // Check if we have a tcp connection for the endpoint, otherwise create one.
    for (TcpSocketWin32 *tcp = network_->tcp_socket_; tcp; tcp = tcp->next_) {
      // After we send 3 handshakes on a tcp socket in a row within a minute,
      // then close and reopen the socket because it seems defunct.
      if (CompareIpAddr(&tcp->endpoint_, &packet->addr) == 0 && tcp->endpoint_protocol_ == packet->protocol) {
        if (is_handshake) {
          uint32 now = (uint32)OsGetMilliseconds();
          uint32 secs = (now - tcp->handshake_timestamp_) >> 10;
          tcp->handshake_timestamp_ += secs * 1024;
          int calc = (secs > (uint32)tcp->handshake_attempts + 25) ? 0 : tcp->handshake_attempts + 25 - secs;
          tcp->handshake_attempts = calc;
          if (calc >= 60) {
            RINFO("Making new Tcp socket due to too many handshake failures");
            tcp->CloseSocket();
            break;
          }
        }
        tcp->WritePacket(exch(packet, Packet_NEXT(packet)));
        goto AGAIN;
      }
    }

    // Drop tcp packet that's for an incoming connection, or packets that are
    // not a handshake.
    if ((packet->protocol & kPacketProtocolIncomingConnection) || !is_handshake) {
      FreePacket(exch(packet, Packet_NEXT(packet)));
      continue;
    }

    // Initialize a new tcp socket and connect to the endpoint
    TcpSocketWin32 *tcp = new TcpSocketWin32(network_, packet_handler_, obfuscator_, false);
    tcp->state_ = TcpSocketWin32::STATE_WANT_CONNECT;
    tcp->endpoint_ = packet->addr;
    tcp->endpoint_protocol_ = kPacketProtocolTcp;
    tcp->handshake_timestamp_ = (uint32)OsGetMilliseconds();
    tcp->WritePacket(exch(packet, Packet_NEXT(packet)));
  }

}

void TcpSocketQueue::OnQueuedItemEvent(QueuedItem *ow, uintptr_t extra) {
  // Runs on the network thread
  wqueue_mutex_.Acquire();
  Packet *packet = wqueue_;
  wqueue_ = NULL;
  wqueue_end_ = &wqueue_;
  wqueue_mutex_.Release();

  TransmitPackets(packet);
}

void TcpSocketQueue::OnQueuedItemDelete(QueuedItem *ow) {

}

void TcpSocketQueue::WritePacket(Packet *packet) {
  packet->queue_next = NULL;
  wqueue_mutex_.Acquire();
  Packet *was_empty = wqueue_;
  *wqueue_end_ = packet;
  wqueue_end_ = &Packet_NEXT(packet);
  wqueue_mutex_.Release();
  if (was_empty == NULL)
    network_->PostQueuedItem(&queued_item_);
}
