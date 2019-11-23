// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.

#pragma once

#include "netapi.h"
#include "network_common.h"
#include "tunsafe_threading.h"

class NetworkWin32;
class PacketProcessor;
class WgPacketObfuscator;

class TcpSocketWin32 : public QueuedItemCallback {
  friend class NetworkWin32;
  friend class TcpSocketQueue;
public:
  explicit TcpSocketWin32(NetworkWin32 *network, PacketProcessor *packet_handler, WgPacketObfuscator *obfuscator, bool is_incoming);
  ~TcpSocketWin32();

  // Write a packet to the TCP socket.
  void WritePacket(Packet *packet);

  // Call from IO completion thread to cancel all outstanding IO
  void CancelAllIO();

  // Call from IO completion thread to run more IO
  void DoIO();

  // Returns true if there's IO still left to run
  bool HasOutstandingIO();

private:
  void DoMoreReads();
  void DoMoreWrites();
  void DoConnect();
  void CloseSocket();
  
  // From OverlappedCallbacks
  virtual void OnQueuedItemEvent(QueuedItem *qi, uintptr_t extra) override;
  virtual void OnQueuedItemDelete(QueuedItem *qi) override;

  // Network subsystem
  NetworkWin32 *network_;

  PacketProcessor *packet_processor_;

  enum {
    STATE_NONE = 0,
    STATE_ERROR = 1,
    STATE_CONNECTING = 2,
    STATE_CONNECTED = 3,
    STATE_WANT_CONNECT = 4,
  };

  uint8 reads_active_;
  uint8 writes_active_;
  uint8 state_;
  uint8 num_wsabuf_;

public:
  uint8 handshake_attempts;
  uint8 endpoint_protocol_;
  uint32 handshake_timestamp_;
private:

  // The handle to the socket
  SOCKET socket_;

  // Packets taken over by the network thread waiting to be written,
  // when these are written we'll start eating from wqueue_
  Packet *pending_writes_;

  // All packets queued for writing on the network thread.
  Packet *wqueue_, **wqueue_end_;

  // Linked list of all TcpSocketWin32 wsockets
  TcpSocketWin32 *next_;

  // Handles packet parsing
  TcpPacketHandler tcp_packet_handler_;

  // An overlapped instance used for the initial Connect() call.
  QueuedItem connect_overlapped_;

  IpAddr endpoint_;

  // Packets currently involved in the wsabuf writing
  enum { kMaxWsaBuf = 32 };
  Packet *packets_in_write_io_[kMaxWsaBuf];
};

class TcpSocketQueue : public QueuedItemCallback {
public:
  explicit TcpSocketQueue(NetworkWin32 *network, WgPacketObfuscator *obfusctor);
  ~TcpSocketQueue();

  void SetPacketHandler(PacketProcessor *packet_handler) { packet_handler_ = packet_handler; }

  virtual void OnQueuedItemEvent(QueuedItem *ow, uintptr_t extra) override;
  virtual void OnQueuedItemDelete(QueuedItem *ow) override;

  void WritePacket(Packet *packet);

private:
  void TransmitPackets(Packet *packet);
  NetworkWin32 *network_;

  // All packets queued for writing on the network thread. Locked by |wqueue_mutex_|
  Packet *wqueue_, **wqueue_end_;

  PacketProcessor *packet_handler_;

  WgPacketObfuscator *obfuscator_;

  // Protects wqueue_
  Mutex wqueue_mutex_;

  // Used for queueing things on the network instance
  QueuedItem queued_item_;
};

