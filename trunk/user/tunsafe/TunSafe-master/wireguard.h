// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once

#include "tunsafe_types.h"
#include "wireguard_proto.h"

// todo: for multithreaded use case need to use atomic ops.
struct WgProcessorStats {
  // The amount of authenticated data received over the wireguard connection.
  uint64 packets_in, data_bytes_in, total_bytes_in;

  // The amount of authenticated data sent across the wireguard connection.
  // |data_bytes_out| holds the actual bytes, while |total_bytes_out|
  // includes wireguard overhead.
  uint64 packets_out, data_bytes_out, total_bytes_out;

  // The amount of invalid data received from the network
  uint64 invalid_packets_in, invalid_bytes_in;

 
  // Error types
  uint32 error_key_id;
  uint32 error_mac;
  uint32 error_duplicate;
  uint32 error_source_addr;
  uint32 error_header;

  // Current speed
  float data_bytes_out_per_second, data_bytes_in_per_second;

  // Timestamp of handshakes
  uint64 first_complete_handshake_timestamp;
  uint64 last_complete_handshake_timestamp;

  // How much saved from header compression
  int64 compression_hdr_saved_in, compression_hdr_saved_out;
  int64 compression_wg_saved_in, compression_wg_saved_out;

  // Number of handshakes received and sent
  // Number of successful handshakes in and out
  uint32 handshakes_in, handshakes_out;
  uint32 handshakes_in_success, handshakes_out_success;

  // Key stuff
  uint8 public_key[32];

  // Address of the endpoint
  IpAddr endpoint;


  // For lost packets calculation, the total # of incoming packets
  uint64 lost_packets_valid;
  // For lost packets calculation, the total # of incoming packets according to seqnr
  uint64 lost_packets_tot;

  uint8 endpoint_protocol;
};

class ProcessorDelegate {
public:
  virtual void OnConnected() = 0;
  virtual void OnConnectionRetry(uint32 attempts) = 0;
};

enum InternetBlockState {
  kBlockInternet_Off = 0,
  kBlockInternet_Route = 1,
  kBlockInternet_Firewall = 2,
  kBlockInternet_Both = 3,
  kBlockInternet_TypeMask = 0xf,

  kBlockInternet_BlockOnDisconnect = 16,
  kBlockInternet_AllowLocalNetworks = 32,

  kBlockInternet_Default = 255,

  kBlockInternet_Active = 256,
};

class WireguardProcessor {
  friend class WgConfig;
public:
  WireguardProcessor(UdpInterface *udp, TunInterface *tun, ProcessorDelegate *procdel);
  ~WireguardProcessor();

  void SetListenPort(int listen_port);
  void SetListenPortTcp(int listen_port);

  void AddDnsServer(const IpAddr &sin);
  bool SetTunAddress(const WgCidrAddr &addr);
  void ClearTunAddress();
  void AddExcludedIp(const WgCidrAddr &cidr_addr);
  void SetMtu(int mtu);
  void SetAddRoutesMode(bool mode);
  void SetDnsBlocking(bool dns_blocking);
  void SetInternetBlocking(InternetBlockState internet_blocking);

  void HandleTunPacket(Packet *packet);
  void HandleUdpPacket(Packet *packet, bool overload);

  // These are the same as above, but instead return the processed packet
  // instead of forwarding it to udp/tun.
  enum PacketResult {
    // The packet is now in use by the wireguard impl and must not be touched
    kPacketResult_InUse = 0,

    // The wireguard impl has processed the packet and it's to be forwarded
    // to the udp / tun layer
    kPacketResult_ForwardUdp = 1,
    kPacketResult_ForwardTun = 2,

    // The wireguard impl does not need the packet and it should be freed
    kPacketResult_Free = 3
  };
  PacketResult HandleTunPacket2(Packet *packet);
  PacketResult HandleUdpPacket2(Packet *packet, bool overload);

  static bool IsMainThreadPacket(Packet *packet);

  void SecondLoop();

  const WgProcessorStats &GetStats();
  void ResetStats();

  bool Start();

  bool ConfigureUdp();
  bool ConfigureTun();

  WgDevice &dev() { return dev_; }
  TunInterface::PrePostCommands &prepost() { return pre_post_; }
  const std::vector<WgCidrAddr> &addr() { return addresses_; }
  void RunAllMainThreadScheduled();

  void ForceSendHandshakeInitiation(WgPeer *peer);

private:
  inline void PrepareOutgoingHandshakePacket(WgPeer *peer, Packet *packet);
  PacketResult WriteAndEncryptPacketToUdp_WillUnlock(WgPeer *peer, Packet *packet);
  void SendHandshakeInitiation(WgPeer *peer);
  void SendKeepalive_Locked(WgPeer *peer);
  void SendQueuedPackets_Locked(WgPeer *peer);

  PacketResult HandleHandshakeInitiationPacket(Packet *packet);
  PacketResult HandleHandshakeResponsePacket(Packet *packet);
  PacketResult HandleHandshakeCookiePacket(Packet *packet);
  PacketResult HandleDataPacket(Packet *packet);
  
  PacketResult HandleAuthenticatedDataPacket_WillUnlock(WgKeypair *keypair, Packet *packet, uint data_size);
  PacketResult HandleShortHeaderFormatPacket(uint32 tag, Packet *packet);
  PacketResult CheckIncomingHandshakeRateLimit(Packet *packet, bool overload);
  bool HandleIcmpv6NeighborSolicitation(const byte *data, size_t data_size);
  void NotifyHandshakeComplete();

  ProcessorDelegate *procdel_;
  TunInterface *tun_;
  UdpInterface *udp_;
  
  uint16 listen_port_;
  uint16 listen_port_tcp_;
  uint16 mtu_;

  bool dns_blocking_;
  uint8 internet_blocking_;
  bool add_routes_mode_;
  bool network_discovery_spoofing_;
  bool did_have_first_handshake_;
  bool is_started_;
  uint8 network_discovery_mac_[6];

  WgDevice dev_;

  WgProcessorStats stats_;

  std::vector<WgCidrAddr> addresses_;
  std::vector<IpAddr> dns_addr_;

  TunInterface::PrePostCommands pre_post_;

  uint64 stats_last_bytes_in_, stats_last_bytes_out_;
  uint64 stats_last_ts_;

  // IPs we want to map to the default route
  std::vector<WgCidrAddr> excluded_ips_;
};

