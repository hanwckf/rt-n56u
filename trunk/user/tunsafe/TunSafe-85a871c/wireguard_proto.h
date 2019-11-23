// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once

#include "tunsafe_types.h"
#include "netapi.h"
#include "tunsafe_config.h"
#include "tunsafe_endian.h"
#include "tunsafe_threading.h"
#include "ip_to_peer_map.h"
#include <vector>
#include <unordered_map>
#include <atomic>
#include <string.h>

#if WITH_BYTELL_HASHMAP
#include "third_party/flat_hash_map/bytell_hash_map.hpp"
#endif  // WITH_BYTELL_HASHMAP

// Threading macros that enable locks only in MT builds
#if WITH_WG_THREADING
#define WG_SCOPED_LOCK(name) ScopedLock scoped_lock(&name)
#define WG_ACQUIRE_LOCK(name) name.Acquire()
#define WG_RELEASE_LOCK(name) name.Release()
#define WG_DECLARE_LOCK(name) Mutex name;
#define WG_DECLARE_RWLOCK(name) ReaderWriterLock name;
#define WG_ACQUIRE_RWLOCK_SHARED(name) name.AcquireShared()
#define WG_RELEASE_RWLOCK_SHARED(name) name.ReleaseShared()
#define WG_ACQUIRE_RWLOCK_EXCLUSIVE(name) name.AcquireExclusive()
#define WG_RELEASE_RWLOCK_EXCLUSIVE(name) name.ReleaseExclusive()
#define WG_SCOPED_RWLOCK_SHARED(name) ScopedLockShared scoped_lock(&name)
#define WG_SCOPED_RWLOCK_EXCLUSIVE(name) ScopedLockExclusive scoped_lock(&name)
#define WG_IF_LOCKS_ENABLED_ELSE(expr, def) (expr)
#else  // WITH_WG_THREADING
#define WG_SCOPED_LOCK(name) 
#define WG_ACQUIRE_LOCK(name) 
#define WG_RELEASE_LOCK(name)
#define WG_DECLARE_LOCK(name)
#define WG_DECLARE_RWLOCK(name)
#define WG_ACQUIRE_RWLOCK_SHARED(name)
#define WG_RELEASE_RWLOCK_SHARED(name)
#define WG_ACQUIRE_RWLOCK_EXCLUSIVE(name)
#define WG_RELEASE_RWLOCK_EXCLUSIVE(name)
#define WG_SCOPED_RWLOCK_SHARED(name)
#define WG_SCOPED_RWLOCK_EXCLUSIVE(name)
#define WG_IF_LOCKS_ENABLED_ELSE(expr, def) (def)
#endif  // WITH_WG_THREADING

// bytell hash is faster but more untested
#if WITH_BYTELL_HASHMAP
#define WG_HASHTABLE_IMPL ska::bytell_hash_map
#else
#define WG_HASHTABLE_IMPL std::unordered_map
#endif

enum ProtocolTimeouts {
  COOKIE_SECRET_MAX_AGE_MS = 120000,
  COOKIE_SECRET_LATENCY_MS = 5000,
  REKEY_TIMEOUT_MS = 5000,
  KEEPALIVE_TIMEOUT_MS = 10000,
  REKEY_AFTER_TIME_MS = 120000,
  REJECT_AFTER_TIME_MS = 180000,
  MIN_HANDSHAKE_INTERVAL_MS = 20,

  HYBRID_TCP_TIMEOUT_MS = 15000,

  // Chosen so that 1500 - 28 - sizeof(handshakeresponse) which means
  // we can use this to probe mtu.
  MAX_SIZE_OF_HANDSHAKE_EXTENSION = 1380,

};

enum ProtocolLimits {
  REJECT_AFTER_MESSAGES = UINT64_MAX - 2048,
  REKEY_AFTER_MESSAGES = UINT64_MAX - 0xffff,

  MAX_HANDSHAKE_ATTEMPTS = 20,
  MAX_QUEUED_PACKETS_PER_PEER = 128,
  MESSAGE_MINIMUM_SIZE = 16,
};

enum MessageType {
  MESSAGE_HANDSHAKE_INITIATION = 1,
  MESSAGE_HANDSHAKE_RESPONSE = 2,
  MESSAGE_HANDSHAKE_COOKIE = 3,
  MESSAGE_DATA = 4,
};

enum MessageFieldSizes {
  WG_COOKIE_LEN = 16,
  WG_COOKIE_NONCE_LEN = 24,
  WG_PUBLIC_KEY_LEN = 32,
  WG_HASH_LEN = 32,
  WG_SYMMETRIC_KEY_LEN = 32,
  WG_MAC_LEN = 16,
  WG_TIMESTAMP_LEN = 12,
  WG_SIPHASH_KEY_LEN = 16,
  WG_PUBLIC_KEY_LEN_BASE64 = 44,
};

enum {
  WG_SHORT_HEADER_BIT = 0x80,
  WG_SHORT_HEADER_KEY_ID_MASK = 0x60,
  WG_SHORT_HEADER_KEY_ID = 0x20,
  WG_SHORT_HEADER_ACK = 0x10,
  WG_SHORT_HEADER_TYPE_MASK = 0x0F,
  WG_SHORT_HEADER_CTR1 = 0x00,
  WG_SHORT_HEADER_CTR2 = 0x01,
  WG_SHORT_HEADER_CTR4 = 0x02,

  WG_ACK_HEADER_COUNTER_MASK = 0x0C,
  WG_ACK_HEADER_COUNTER_NONE = 0x00,
  WG_ACK_HEADER_COUNTER_2 = 0x04,
  WG_ACK_HEADER_COUNTER_4 = 0x08,
  WG_ACK_HEADER_COUNTER_6 = 0x0C,

  WG_ACK_HEADER_KEY_MASK = 3,
};


struct MessageMacs {
  uint8 mac1[WG_COOKIE_LEN];
  uint8 mac2[WG_COOKIE_LEN];
};
STATIC_ASSERT(sizeof(MessageMacs) == 32, MessageMacs_wrong_size);

struct MessageHandshakeInitiation {
  uint32 type;
  uint32 sender_key_id;
  uint8 ephemeral[WG_PUBLIC_KEY_LEN];
  uint8 static_enc[WG_PUBLIC_KEY_LEN + WG_MAC_LEN];
  uint8 timestamp_enc[WG_TIMESTAMP_LEN + WG_MAC_LEN];
  MessageMacs mac;
};
STATIC_ASSERT(sizeof(MessageHandshakeInitiation) == 148, MessageHandshakeInitiation_wrong_size);

// Format of variable length payload.
// 1 byte type
// 1 byte length
// <payload>

struct MessageHandshakeResponse {
  uint32 type;
  uint32 sender_key_id;
  uint32 receiver_key_id;
  uint8 ephemeral[WG_PUBLIC_KEY_LEN];
  uint8 empty_enc[WG_MAC_LEN];
  MessageMacs mac;
};
STATIC_ASSERT(sizeof(MessageHandshakeResponse) == 92, MessageHandshakeResponse_wrong_size);

struct MessageHandshakeCookie {
  uint32 type;
  uint32 receiver_key_id;
  uint8 nonce[WG_COOKIE_NONCE_LEN];
  uint8 cookie_enc[WG_COOKIE_LEN + WG_MAC_LEN];
};
STATIC_ASSERT(sizeof(MessageHandshakeCookie) == 64, MessageHandshakeCookie_wrong_size);

struct MessageData {
  uint32 type;
  uint32 receiver_id;
  uint64 counter;
};
STATIC_ASSERT(sizeof(MessageData) == 16, MessageData_wrong_size);

enum {
  kExtensionType_Padding = 0x00,
  kExtensionType_Booleans = 0x01,
  kExtensionType_CipherSuites = 0x02,
  kExtensionType_CipherSuitesPrio = 0x03,

  // The standard wireguard chacha
  EXT_CIPHER_SUITE_CHACHA20POLY1305 = 0x00,
  // AES GCM 128 bit
  EXT_CIPHER_SUITE_AES128_GCM = 0x01,
  // AES GCM 256 bit
  EXT_CIPHER_SUITE_AES256_GCM = 0x02,
  // Same as CHACHA20POLY1305 but without the encryption step
  EXT_CIPHER_SUITE_NONE_POLY1305 = 0x03,

  EXT_CIPHER_SUITE_COUNT = 4,
};

enum {
  WG_FEATURES_COUNT = 7,
  WG_FEATURE_ID_SHORT_HEADER = 0,    // Supports short headers
  WG_FEATURE_ID_SHORT_MAC = 1,       // Supports 8-byte MAC
  WG_FEATURE_ID_IPZIP = 2,           // Using ipzip
  WG_FEATURE_ID_SKIP_KEYID_IN = 4,   // Skip keyid for incoming packets
  WG_FEATURE_ID_SKIP_KEYID_OUT = 5,  // Skip keyid for outgoing packets
  WG_FEATURE_HYBRID_TCP = 6,         // Use hybrid-tcp mode
};

enum {
  WG_BOOLEAN_FEATURE_OFF = 0x0,
  WG_BOOLEAN_FEATURE_SUPPORTS = 0x1,
  WG_BOOLEAN_FEATURE_WANTS = 0x2,
  WG_BOOLEAN_FEATURE_ENFORCES = 0x3,
};

struct WgKeypair;
class WgPeer;

class WgRateLimit {
public:
  WgRateLimit();

  struct RateLimitResult {
    uint8 *value_ptr;
    uint8 new_value;
    uint8 is_ok;

    bool is_rate_limited() { return !is_ok; }
    bool is_first_ip() { return new_value == 1; }
  };

  RateLimitResult CheckRateLimit(uint64 ip);

  void CommitResult(const RateLimitResult &rr) { *rr.value_ptr = rr.new_value; if (used_rate_limit_++ == TOTAL_PACKETS_PER_SEC) packets_per_sec_ = (packets_per_sec_ + 1) >> 1; }

  void Periodic(uint32 s[5]);

  bool is_used() { return used_rate_limit_ != 0 || packets_per_sec_ != PACKETS_PER_SEC; }
private:
  uint8 *bin1_, *bin2_;
  uint32 rand_, rand_xor_;
  uint32 packets_per_sec_, used_rate_limit_;
  uint64 key1_[2], key2_[2];
  enum {
    BINSIZE = 4096,
    PACKETS_PER_SEC = 25,
    PACKET_ACCUM = 100,
    TOTAL_PACKETS_PER_SEC = 25000,
  };
  uint8 bins_[2][BINSIZE];
};

struct WgAddrEntry {
  struct IpPort {
    uint8 bytes[20];

    friend bool operator==(const IpPort &a, const IpPort &b) {
      uint64 rv = Read64(a.bytes) ^ Read64(b.bytes);
      rv |= Read64(a.bytes + 8) ^ Read64(b.bytes + 8);
      rv |= Read32(a.bytes + 16) ^ Read32(b.bytes + 16);
      return (rv == 0);
    }
  };

  struct IpPortHasher {
    size_t operator()(const IpPort &a) const;
  };

  // The id of the addr entry, so we can delete ourselves
  IpPort addr_entry_id;

  // This entry gets erased when there's no longer any key pointing at it.
  uint8 ref_count;

  // Index of the next slot 0-2 where we'll insert the next key.
  uint8 next_slot;

  // Ensure there's at least 1 minute between we allow registering
  // a new key in this table. This means that each key will have
  // a life time of at least 3 minutes.
  uint64 time_of_last_insertion;

  // The three keys.
  WgKeypair *keys[3];

  WgAddrEntry(const IpPort &addr_entry_id) 
      : addr_entry_id(addr_entry_id), ref_count(0), next_slot(0), time_of_last_insertion(0) {
    keys[0] = keys[1] = keys[2] = NULL;
  }

};

union WgPublicKey {
  uint8 bytes[WG_PUBLIC_KEY_LEN];
  uint64 u64[WG_PUBLIC_KEY_LEN / 8];
  friend bool operator==(const WgPublicKey &a, const WgPublicKey &b) {
    return memcmp(a.bytes, b.bytes, WG_PUBLIC_KEY_LEN) == 0;
  }
};

struct WgPublicKeyHasher {
  size_t operator()(const WgPublicKey&a) const;
};

class WgCompressHandler {
public:
  virtual ~WgCompressHandler() {}

  enum CompressState {
    COMPRESS_FAIL = -1,
    COMPRESS_NO = 0,
    COMPRESS_YES = 1,
  };

  // Compress a packet. 
  virtual CompressState Compress(Packet *packet);

  virtual CompressState Decompress(Packet *packet);
};

// Can be used to customize the behavior of the wireguard impl
class WgPlugin {
public:
  virtual ~WgPlugin() {}

  // This is called from the main thread whenever a public key was not found in the WgDevice,
  // return true to try again or false to fail. The packet can be copied and saved
  // to resume a handshake later on.
  virtual bool HandleUnknownPeerId(uint8 public_key[WG_PUBLIC_KEY_LEN], Packet *packet) = 0;

  // For handling unknown settings during config parsing
  virtual bool OnUnknownInterfaceSetting(const char *key, const char *value) = 0;
  virtual bool OnUnknownPeerSetting(WgPeer *peer, const char *key, const char *value) = 0;

  // Called after settings have been completely parsed, the plugin may modify the state
  virtual bool OnAfterSettingsParsed() = 0;

  // Returns true if we want to perform a handshake for this peer.
  virtual bool WantHandshake(WgPeer *peer) = 0;

  enum {
    kHandshakeResponseDrop = 0xffffffff,
    kHandshakeResponseFail = 0x80000000
  };
  // Called before handshake initiation is sent out. Can write extra headers. Can't drop packets.
  virtual uint32 OnHandshake0(WgPeer *peer, uint8 *extout, uint32 extout_size, const uint8 salt[WG_PUBLIC_KEY_LEN]) = 0;
  // Called after handshake initiation is parsed, but before handshake response is sent.
  // Packet can be dropped or keypair failed.
  virtual uint32 OnHandshake1(WgPeer *peer, const uint8 *ext, uint32 ext_size, const uint8 salt_in[WG_PUBLIC_KEY_LEN], uint8 *extout, uint32 extout_size, const uint8 salt_out[WG_PUBLIC_KEY_LEN]) = 0;
  // Called when handshake response is parsed
  virtual uint32 OnHandshake2(WgPeer *peer, const uint8 *ext, uint32 ext_size, const uint8 salt[WG_PUBLIC_KEY_LEN]) = 0;

  // Called right before an outgoing non-data packet is sent out, but before it's scrambled.
  virtual void OnOutgoingHandshakePacket(WgPeer *peer, Packet *packet) = 0;
};


// This class is used for scrambing / unscrambling of wireguard UDP/TCP packets,
// including adding random bytes at the end of the non-data packets.
class WgPacketObfuscator {
public:
  WgPacketObfuscator() : enabled_(false), obfuscate_tcp_(-1) {}

  bool enabled() { return enabled_; }
  void ObfuscatePacket(Packet *packet);
  void DeobfuscatePacket(Packet *packet);

  void SetKey(const uint8 *key, size_t len);

  const uint8 *key() { return (uint8*)key_; }

  int obfuscate_tcp() { return obfuscate_tcp_; }
  void set_obfuscate_tcp(int v) { obfuscate_tcp_ = v; }

  static size_t InsertRandomBytesIntoPacket(uint8 *data, size_t data_size);

private:
  void ScrambleUnscramble(uint8 *data, size_t data_size);

  // Whether packet obfuscation is enabled
  bool enabled_;

  // Type of obfuscation for tcp
  int obfuscate_tcp_;

  // Siphash keys for packet scrambling
  uint64 key_[4];
};

class WgDevice {
  friend class WgPeer;
  friend class WireguardProcessor;
  friend class WgConfig;
public:

  WgDevice();
  ~WgDevice();

  // Configure with the private key, precompute all internal keys etc.
  void SetPrivateKey(const uint8 private_key[WG_PUBLIC_KEY_LEN]);

  // Create a new peer
  WgPeer *AddPeer();

  // Remove all peers
  void RemoveAllPeers();

  // Check whether Mac1 appears to be valid
  bool CheckCookieMac1(Packet *packet);

  // Check whether Mac2 appears to be valid, this also uses the remote ip address
  bool CheckCookieMac2(Packet *packet);

  void CreateCookieMessage(MessageHandshakeCookie *dst, Packet *packet, uint32 remote_key_id);
  void SecondLoop(uint64 now);

  IpToPeerMap &ip_to_peer_map() { return ip_to_peer_map_; }
  WgPeer *first_peer() { return peers_; }
  const uint8 *public_key() const { return s_pub_; }
  WgRateLimit *rate_limiter() { return &rate_limiter_; }
  bool is_private_key_initialized() { return is_private_key_initialized_; }

  void SetCurrentThreadAsMainThread() { main_thread_id_ = GetCurrentThreadId(); }

  bool IsMainThread() { return CurrentThreadIdEquals(main_thread_id_); }
  bool IsMainOrDataThread() { return CurrentThreadIdEquals(main_thread_id_) || WG_IF_LOCKS_ENABLED_ELSE(delayed_delete_.enabled(), false);  }

  void SetPlugin(WgPlugin *del) { plugin_ = del; }
  WgPlugin *plugin() { return plugin_; }

  MultithreadedDelayedDelete *GetDelayedDelete() { return &delayed_delete_; }

  WgPacketObfuscator &packet_obfuscator() { return packet_obfuscator_; }

private:
  std::pair<WgPeer*, WgKeypair*> *LookupPeerInKeyIdLookup(uint32 key_id);
  WgKeypair *LookupKeypairByKeyId(uint32 key_id);

  void UpdateKeypairAddrEntry_Locked(const IpAddr &addr, WgKeypair *keypair);
  WgKeypair *LookupKeypairInAddrEntryMap(const IpAddr &addr, uint32 slot);
  // Return the peer matching the |public_key| or NULL
  WgPeer *GetPeerFromPublicKey(const WgPublicKey &pubkey);
  // Create a cookie by inspecting the source address of the |packet|
  void MakeCookie(uint8 cookie[WG_COOKIE_LEN], Packet *packet);
  // Insert a new entry in |key_id_lookup_|
  uint32 InsertInKeyIdLookup(WgPeer *peer, WgKeypair *kp);
  // Get a random number
  uint32 GetRandomNumber();

  void EraseKeypairAddrEntry_Locked(WgKeypair *kp);

  // Maps IP addresses to peers
  IpToPeerMap ip_to_peer_map_;

  // This lock protects |ip_to_peer_map_|.
  WG_DECLARE_RWLOCK(ip_to_peer_map_lock_);
   
  // For enumerating all peers
  WgPeer *peers_, **last_peer_ptr_;

  // For hooking
  WgPlugin *plugin_;


  // Keypair IDs are generated randomly by us so no point in wasting cycles on
  // hashing the random value.
  struct KeyIdHasher {
    size_t operator()(uint32 v) const { return v;  }
  };
  
  // Lock that protects key_id_lookup_
  WG_DECLARE_RWLOCK(key_id_lookup_lock_);
  // Mapping from key-id to either an active keypair (if keypair is non-NULL),
  // or to a handshake.
  WG_HASHTABLE_IMPL<uint32, std::pair<WgPeer*, WgKeypair*>, KeyIdHasher> key_id_lookup_;

  // Mapping from IPV4 IP/PORT to WgPeer*, so we can find the peer when a key id is
  // not explicitly included. Use void* here so we can reuse the same template instance.
  WG_HASHTABLE_IMPL<WgAddrEntry::IpPort, void*, WgAddrEntry::IpPortHasher> addr_entry_lookup_;
  WG_DECLARE_RWLOCK(addr_entry_lookup_lock_);

  // Mapping from peer id to peer. This may be accessed only from MT.
  WG_HASHTABLE_IMPL<WgPublicKey, WgPeer*, WgPublicKeyHasher> peer_id_lookup_;
  // Queue of things scheduled to run on the main thread.
  WG_DECLARE_LOCK(main_thread_scheduled_lock_);
  WgPeer *main_thread_scheduled_, **main_thread_scheduled_last_;

  // Counter for generating new indices in |keypair_lookup_|
  uint8 next_rng_slot_;

  // Whether a private key has been setup for the device
  bool is_private_key_initialized_;

  ThreadId main_thread_id_;

  uint64 low_resolution_timestamp_;

  uint64 cookie_secret_timestamp_;
  uint8 cookie_secret_[WG_HASH_LEN];
  uint8 s_priv_[WG_PUBLIC_KEY_LEN];
  uint8 s_pub_[WG_PUBLIC_KEY_LEN];


  uint8 precomputed_cookie_key_[WG_SYMMETRIC_KEY_LEN];
  uint8 precomputed_mac1_key_[WG_SYMMETRIC_KEY_LEN];

  uint64 random_number_input_[WG_HASH_LEN / 8 + 1];
  uint32 random_number_output_[WG_HASH_LEN / 4];

  WgPacketObfuscator packet_obfuscator_;

  WgRateLimit rate_limiter_;

  // For defering deletes until all worker threads are guaranteed not to use an object.
  MultithreadedDelayedDelete delayed_delete_;
};

// Allows associating extradata with peers that can be used by plugins etc.
class WgPeerExtraData {
public:
  // This is called when the peer is destroyed.
  virtual void OnPeerDestroy() = 0;
};

// State for peer
class WgPeer {
  friend class WgDevice;
  friend class WireguardProcessor;
  friend class WgConfig;
public:
  explicit WgPeer(WgDevice *dev);
  ~WgPeer();

  void SetPublicKey(const WgPublicKey &spub);
  void SetPresharedKey(const uint8 preshared_key[WG_SYMMETRIC_KEY_LEN]);
  bool SetPersistentKeepalive(int persistent_keepalive_secs);
  void SetEndpoint(int endpoint_proto, const IpAddr &sin);
  void SetAllowMulticast(bool allow);

  void SetFeature(int feature, uint8 value);
  bool AddCipher(int cipher);
  void SetCipherPrio(bool prio) { cipher_prio_ = prio; }
  bool AddIp(const WgCidrAddr &cidr_addr);
  void RemoveAllIps();

  static WgPeer *ParseMessageHandshakeInitiation(WgDevice *dev, Packet *packet);
  static WgPeer *ParseMessageHandshakeResponse(WgDevice *dev, const Packet *packet);
  static void ParseMessageHandshakeCookie(WgDevice *dev, const MessageHandshakeCookie *src);
  void CreateMessageHandshakeInitiation(Packet *packet);
  bool CheckSwitchToNextKey_Locked(WgKeypair *keypair);
  void RemovePeer();
  bool CheckHandshakeRateLimit();

  // Timer notifications
  void OnDataSent();
  void OnKeepaliveSent();
  void OnDataReceived();
  void OnKeepaliveReceived();
  void OnHandshakeInitSent();
  void OnHandshakeAuthComplete();
  void OnHandshakeFullyComplete();

  enum {
    ACTION_SEND_KEEPALIVE = 1,
    ACTION_SEND_HANDSHAKE = 2,
  };
  uint32 CheckTimeouts_Locked(uint64 now);

  void AddPacketToPeerQueue_Locked(Packet *packet);
  bool IsPeerLocked() { return WG_IF_LOCKS_ENABLED_ELSE(mutex_.IsLocked(), true); }

  const IpAddr &endpoint() const { return endpoint_; }
  uint8 endpoint_protocol() const { return endpoint_protocol_; }
  WgPeer *next_peer() { return next_peer_; }

  WgPeerExtraData *extradata() { return peer_extra_data_; }
  void SetExtradata(WgPeerExtraData *ex) { peer_extra_data_ = ex; }
  WgDevice *dev() { return dev_; }

  const uint8 *epriv() { return hs_.e_priv; }

private:
  bool ParseExtendedHandshake(WgKeypair *keypair, const uint8 *data, size_t data_size);
  static WgKeypair *CreateNewKeypair(bool is_initiator, const uint8 key[WG_HASH_LEN], uint32 send_key_id);
  void WriteMacToPacket(const uint8 *data, MessageMacs *mac);
  void CheckAndUpdateTimeOfNextKeyEvent(uint64 now);
  static void DeleteKeypair(WgKeypair **kp);
  static void DelayedDelete(void *x);
  int WriteHandshakeExtension(uint8 *dst, WgKeypair *keypair);
  void InsertKeypairInPeer_Locked(WgKeypair *keypair);
  void ClearKeys_Locked();
  void ClearHandshake_Locked();
  void ClearPacketQueue_Locked();
  void ScheduleNewHandshake();
  bool IsTransientDataEndpointActive();
  
  WgDevice *dev_;
  WgPeer *next_peer_;

  // Keypairs, |curr_keypair_| is the used one, the other ones are
  // the old ones and the next one.
  WgKeypair *curr_keypair_, *prev_keypair_, *next_keypair_;

  // Protects shared variables of the WgPeer
  WG_DECLARE_LOCK(mutex_);

  // Timestamp when the next key related event is going to occur.
  uint64 time_of_next_key_event_;

  // For timer management
  uint32 timers_;

  // Holds the entry into the key id table during handshake - mt only.
  uint32 local_key_id_during_hs_;

  enum {
    kMainThreadScheduled_ScheduleHandshake = 1,
  };
  std::atomic<uint32> main_thread_scheduled_;
  WgPeer *main_thread_scheduled_next_;

  // The broadcast address of the IPv4 network, used to block broadcast traffic
  // from being sent out over the VPN link.
  uint32 ipv4_broadcast_addr_;

  // Whether any data was sent since the keepalive timer was set
  bool pending_keepalive_;

  // Whether to change the endpoint on incoming packets.
  bool allow_endpoint_change_;

  // Whether we've sent a mac to the peer so we may expect a cookie reply back.
  bool expect_cookie_reply_;

  // Whether we want to route incoming multicast/broadcast traffic to this peer.
  bool allow_multicast_through_peer_;

  // Whether |mac2_cookie_| is valid.
  bool has_mac2_cookie_;

  // Whether the WgPeer has been deleted (i.e. RemovePeer has been called),
  // and will be deleted as soon as the threads sync.
  bool marked_for_delete_;

  // Number of handshakes made so far, when this gets too high we stop connecting.
  uint8 handshake_attempts_;

  // What's the protocol of the currently configured endpoint
  uint8 endpoint_protocol_, data_endpoint_protocol_;

  // Which features are enabled for this peer?
  uint8 features_[WG_FEATURES_COUNT];

  // Queue of packets that will get sent once handshake finishes
  uint8 num_queued_packets_;
  Packet *first_queued_packet_, **last_queued_packet_ptr_;

  // For statistics
  uint64 last_handshake_init_timestamp_;
  uint64 last_complete_handskake_timestamp_;

  // Timestamp to detect flooding of handshakes
  uint64 last_handshake_init_recv_timestamp_;  // main thread only

  // Address of peer
  IpAddr endpoint_;

  // Alternative endpoint. This is used in hybrid tcp mode to hold the 
  // udp endpoint.
  IpAddr data_endpoint_;
  
  // Number of handshake attempts since last successful handshake
  uint32 total_handshake_attempts_;

  // For dynamic ciphers, holds the list of supported ciphers.
  enum { MAX_CIPHERS = 4 };
  uint8 cipher_prio_;
  uint8 num_ciphers_;
  uint8 ciphers_[MAX_CIPHERS];

  uint32 keepalive_timeout_ms_; // Set to KEEPALIVE_TIMEOUT_MS

  uint32 timer_value_[6];
  
  uint64 rx_bytes_;
  uint64 tx_bytes_;

  WgPeerExtraData *peer_extra_data_;

  // Handshake state that gets setup in |CreateMessageHandshakeInitiation| and used in
  // the response.
  struct HandshakeState {
    // Hash
    uint8 hi[WG_HASH_LEN];
    // Chaining key
    uint8 ci[WG_HASH_LEN];
    // Private ephemeral
    uint8 e_priv[WG_PUBLIC_KEY_LEN];
  };
  HandshakeState hs_;
  // Remote's static public key - init only.
  WgPublicKey s_remote_;
  // Remote's preshared key - init only.
  uint8 preshared_key_[WG_SYMMETRIC_KEY_LEN];
  // Precomputed DH(spriv_local, spub_remote) - init only.
  uint8 s_priv_pub_[WG_PUBLIC_KEY_LEN];
  // The most recent seen timestamp, only accept higher timestamps - mt only.
  uint8 last_timestamp_[WG_TIMESTAMP_LEN]; 
  // Precomputed key for decrypting cookies from the peer - init only.
  uint8 precomputed_cookie_key_[WG_SYMMETRIC_KEY_LEN];
  // Precomputed key for sending MACs to the peer - init only.
  uint8 precomputed_mac1_key_[WG_SYMMETRIC_KEY_LEN];
  // The last mac value sent, required to make cookies - mt only.
  uint8 sent_mac1_[WG_COOKIE_LEN];
  // The mac2 cookie that gets appended to outgoing packets
  uint8 mac2_cookie_[WG_COOKIE_LEN];
  // The timestamp of the mac2 cookie
  uint64 mac2_cookie_timestamp_;
  int persistent_keepalive_ms_;

  // Allowed ips
  std::vector<WgCidrAddr> allowed_ips_;
};

// RFC6479 - IPsec Anti-Replay Algorithm without Bit Shifting
class ReplayDetector {
public:
  ReplayDetector();
  ~ReplayDetector();

  bool CheckReplay(uint64 other);
  enum {
    BITS_PER_ENTRY = 32,
    WINDOW_SIZE = 2048 - BITS_PER_ENTRY,
    BITMAP_SIZE = WINDOW_SIZE / BITS_PER_ENTRY + 1,
    BITMAP_MASK = BITMAP_SIZE - 1,
  };

  const uint64 expected_seq_nr() const { return expected_seq_nr_; }

private:
  std::atomic<uint64> expected_seq_nr_;
  uint32 bitmap_[BITMAP_SIZE];
};

struct AesGcm128StaticContext;

struct WgKeypair {
  WgPeer *peer;

  // If the key has an addr entry mapping,
  // then this points at it.
  WgAddrEntry *addr_entry;
  // The slot in the addr entry where the key is registered.
  uint8 addr_entry_slot;

  enum {
    KEY_INVALID = 0,
    KEY_VALID = 1,
    KEY_WANT_REFRESH = 2,
    KEY_DID_REFRESH = 3,
  };
  // True if i'm the initiator of the key exchange
  bool is_initiator;

  // True if we saved the peer's address in our table recently,
  // avoids doing it too much
  bool did_attempt_remember_ip_port;

  // Which features are enabled
  bool enabled_features[WG_FEATURES_COUNT];

  // True if we want to notify the sender about that it can use a short key.
  uint8 broadcast_short_key;

  // Index of the short key index that we can use for outgoing packets.
  uint8 can_use_short_key_for_outgoing;

  // Whether the key is valid or needs refresh for receives
  uint8 recv_key_state;
  // Whether the key is valid or needs refresh for sends
  uint8 send_key_state;

  // Length of authentication tag
  uint8 auth_tag_length;

  // Cipher suite
  uint8 cipher_suite;

  // Id of the key in my map. (MainThread)
  uint32 local_key_id;
  // Id of the key in their map
  uint32 remote_key_id;
  // Used to send out acks.
  uint32 send_ack_ctr;
  // The timestamp of when the key was created, to be able to expire it
  uint64 key_timestamp;
  // The highest acked send_ctr value
  uint64 send_ctr_acked;
  // Used to detect incoming packet loss
  uint64 incoming_packet_count;
  // Counter value for chacha20 for outgoing packets
  uint64 send_ctr;
  // The key used for chacha20 encryption
  uint8 send_key[WG_SYMMETRIC_KEY_LEN];
  // The key used for chacha20 decryption
  uint8 recv_key[WG_SYMMETRIC_KEY_LEN];

  // Used when less than 16-byte mac is enabled to hash the hmac into 64 bits.
  uint64 compress_mac_keys[2][2];

  AesGcm128StaticContext *aes_gcm128_context_;

  WgCompressHandler *compress_handler_;

  // -- all up to this point is initialized to zero
  // For replay detection of incoming packets
  ReplayDetector replay_detector;

};

void WgKeypairEncryptPayload(uint8 *dst, const size_t src_len,
    const uint8 *ad, const size_t ad_len,
    const uint64 nonce, WgKeypair *keypair);

bool WgKeypairDecryptPayload(uint8 *dst, const size_t src_len,
    const uint8 *ad, const size_t ad_len,
    const uint64 nonce, WgKeypair *keypair);

struct WgExtensionHooksDefault {
  static uint32 GetIpv4Target(Packet *packet, uint8 *data) { return ReadBE32(data + 16); }
  static void OnPeerIncomingUdp(WgPeer *peer, const Packet *packet, uint data_size) { }
  static void OnPeerOutgoingUdp(WgPeer *peer, Packet *packet) { }
  static bool DisableSourceAddressVerification(WgPeer *peer) { return false; }
};

#ifndef WG_EXTENSION_HOOKS
#define WG_EXTENSION_HOOKS WgExtensionHooksDefault
#endif  // WG_EXTENSION_HOOKS

