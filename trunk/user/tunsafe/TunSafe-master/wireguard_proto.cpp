// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "stdafx.h"
#include "wireguard_proto.h"
#include "crypto/chacha20poly1305.h"
#include "crypto/blake2s/blake2s.h"
#include "crypto/curve25519/curve25519-donna.h"
#include "crypto/aesgcm/aes.h"
#include "crypto/siphash/siphash.h"
#include "tunsafe_endian.h"
#include "util.h"
#include "crypto_ops.h"
#include "bit_ops.h"
#include "tunsafe_cpu.h"
#include <algorithm>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

enum {
  kTunsafeExtensionClientID = ('T' | 'S' << 8)
};

static const uint8 kLabelCookie[] = {'c', 'o', 'o', 'k', 'i', 'e', '-', '-'};
static const uint8 kLabelMac1[] = {'m', 'a', 'c', '1', '-', '-', '-', '-'};
static const uint8 kWgInitHash[WG_HASH_LEN] = {0x22,0x11,0xb3,0x61,0x08,0x1a,0xc5,0x66,0x69,0x12,0x43,0xdb,0x45,0x8a,0xd5,0x32,0x2d,0x9c,0x6c,0x66,0x22,0x93,0xe8,0xb7,0x0e,0xe1,0x9c,0x65,0xba,0x07,0x9e,0xf3};
static const uint8 kWgInitChainingKey[WG_HASH_LEN] = {0x60,0xe2,0x6d,0xae,0xf3,0x27,0xef,0xc0,0x2e,0xc3,0x35,0xe2,0xa0,0x25,0xd2,0xd0,0x16,0xeb,0x42,0x06,0xf8,0x72,0x77,0xf5,0x2d,0x38,0xd1,0x98,0x8b,0x78,0xcd,0x36};

ReplayDetector::ReplayDetector() {
  expected_seq_nr_ = 0;
  memset(bitmap_, 0, sizeof(bitmap_));
}

ReplayDetector::~ReplayDetector() {
}

bool ReplayDetector::CheckReplay(uint64 seq_nr) {
  uint64 slot = seq_nr / BITS_PER_ENTRY;
  uint64 expected_seq_nr = expected_seq_nr_;
  if (seq_nr >= expected_seq_nr) {
    uint64 prev_slot = (expected_seq_nr + BITS_PER_ENTRY - 1) / BITS_PER_ENTRY - 1, n;
    if ((n = slot - prev_slot) != 0) {
      size_t nn = (size_t)std::min<uint64>(n, BITMAP_SIZE);
      do {
        bitmap_[(prev_slot + nn) & BITMAP_MASK] = 0;
      } while (--nn);
    }
    expected_seq_nr_ = seq_nr + 1;
  } else if (seq_nr + WINDOW_SIZE <= expected_seq_nr) {
    return false;
  }
  uint32 mask = 1 << (seq_nr & (BITS_PER_ENTRY - 1)), prev;
  prev = bitmap_[slot & BITMAP_MASK];
  bitmap_[slot & BITMAP_MASK] = prev | mask;
  return (prev & mask) == 0;
}

WgDevice::WgDevice() {
  peers_ = NULL;
  last_peer_ptr_ = &peers_;
  plugin_ = NULL;
  is_private_key_initialized_ = false;
  next_rng_slot_ = 0;
  main_thread_scheduled_ = NULL;
  main_thread_scheduled_last_ = &main_thread_scheduled_;

  low_resolution_timestamp_ = cookie_secret_timestamp_ = OsGetMilliseconds();
  OsGetRandomBytes(cookie_secret_, sizeof(cookie_secret_));
  OsGetRandomBytes((uint8*)random_number_input_, sizeof(random_number_input_));
  main_thread_id_ = GetCurrentThreadId();

  memset(s_priv_, 0, sizeof(s_priv_));
  memset(s_pub_, 0, sizeof(s_pub_));
}

WgDevice::~WgDevice() {
  assert(IsMainThread());
  RemoveAllPeers();
}

void WgDevice::SecondLoop(uint64 now) {
  assert(IsMainThread());

  low_resolution_timestamp_ = now;
  if (rate_limiter_.is_used()) {
    uint32 k[5];
    for (size_t i = 0; i < ARRAY_SIZE(k); i++)
      k[i] = GetRandomNumber();
    rate_limiter_.Periodic(k);
  }
}

uint32 WgDevice::InsertInKeyIdLookup(WgPeer *peer, WgKeypair *kp) {
  assert(IsMainThread());
  assert(peer);
  for (;;) {
    uint32 v = GetRandomNumber();
    if (v == 0)
      continue;

    // Take the exclusive lock since we're modifying it.
    WG_SCOPED_RWLOCK_EXCLUSIVE(key_id_lookup_lock_);

    std::pair<WgPeer*, WgKeypair*> &peer_and_keypair = key_id_lookup_[v];
    if (peer_and_keypair.first == NULL) {
      peer_and_keypair = std::make_pair(peer, kp);
      uint32 &x = (kp ? kp->local_key_id : peer->local_key_id_during_hs_);
      uint32 old = x;
      x = v;
      if (old)
        key_id_lookup_.erase(old);
      return v;
    }
  }
}

std::pair<WgPeer*, WgKeypair*> *WgDevice::LookupPeerInKeyIdLookup(uint32 key_id) {
  // This function is only ever called by the main thread, so no need to lock,
  // since the main thread is the only mutator.
  assert(IsMainThread());
  auto it = key_id_lookup_.find(key_id);
  return (it != key_id_lookup_.end() && it->second.second == NULL) ? &it->second : NULL;
}

WgKeypair *WgDevice::LookupKeypairByKeyId(uint32 key_id) {
  // This function can be called from any thread, so make sure to 
  // lock using the shared lock.
  WG_SCOPED_RWLOCK_SHARED(key_id_lookup_lock_);
  auto it = key_id_lookup_.find(key_id);
  return (it != key_id_lookup_.end()) ? it->second.second : NULL;
}

uint32 WgDevice::GetRandomNumber() {
  assert(IsMainThread());
  size_t slot;
  if ((slot = next_rng_slot_) == 0) {
    blake2s(random_number_output_, sizeof(random_number_output_), random_number_input_, sizeof(random_number_input_), NULL,  0);
    random_number_input_[0]++;
    slot = BLAKE2S_OUTBYTES / 4;
  }
  next_rng_slot_ = (uint8) --slot;
  return random_number_output_[slot];
}

static void BlakeX2(uint8 *dst, size_t dst_size, const uint8 *a, size_t a_size, const uint8 *b, size_t b_size) {
  blake2s_state b2s;
  blake2s_init(&b2s, dst_size);
  blake2s_update(&b2s, a, a_size);
  blake2s_update(&b2s, b, b_size);
  blake2s_final(&b2s, dst, dst_size);
}

static inline void BlakeMix(uint8 dst[WG_HASH_LEN], const uint8 *a, size_t a_size) {
  BlakeX2(dst, WG_HASH_LEN, dst, WG_HASH_LEN, a, a_size);
}

static inline void ComputeHKDF2DH(uint8 ci[WG_HASH_LEN], uint8 k[WG_SYMMETRIC_KEY_LEN], const uint8 priv[WG_PUBLIC_KEY_LEN], const uint8 pub[WG_PUBLIC_KEY_LEN]) {
  uint8 dh[WG_PUBLIC_KEY_LEN];
  curve25519_donna(dh, priv, pub);
  blake2s_hkdf(ci, WG_HASH_LEN, k, WG_SYMMETRIC_KEY_LEN, NULL, 32,  dh, sizeof(dh), ci, WG_HASH_LEN);
  memzero_crypto(dh, sizeof(dh));
}

void WgDevice::SetPrivateKey(const uint8 private_key[WG_PUBLIC_KEY_LEN]) {
  assert(IsMainThread());
  // Derive the public key from the private key.
  memcpy(s_priv_, private_key, sizeof(s_priv_));
  curve25519_donna(s_pub_, s_priv_, kCurve25519Basepoint);

  // Precompute: precomputed_cookie_label_hash_ := HASH(LABEL-COOKIE || Spub_m)
  //             precomputed_label_mac1_hash_ := HASH(MAC1-COOKIE || Spub_m)
  BlakeX2(precomputed_cookie_key_, sizeof(precomputed_cookie_key_),
          kLabelCookie, sizeof(kLabelCookie), s_pub_, sizeof(s_pub_));
  BlakeX2(precomputed_mac1_key_, sizeof(precomputed_mac1_key_),
          kLabelMac1, sizeof(kLabelMac1), s_pub_, sizeof(s_pub_));

  is_private_key_initialized_ = true;

  // Recompute peer data because it depends on my privkey
  for (WgPeer *peer = peers_; peer; peer = peer->next_peer_)
    peer->SetPublicKey(peer->s_remote_);
}

WgPeer *WgDevice::AddPeer() {
  assert(IsMainThread());
  WgPeer *peer = new WgPeer(this);
  return peer;
}

void WgDevice::RemoveAllPeers() {
  assert(IsMainThread());
  while (peers_)
    peers_->RemovePeer();
}

WgPeer *WgDevice::GetPeerFromPublicKey(const WgPublicKey &pubkey) {
  assert(IsMainThread());

  auto it = peer_id_lookup_.find(pubkey);
  return (it != peer_id_lookup_.end()) ? it->second : NULL;
}

bool WgDevice::CheckCookieMac1(Packet *packet) {
  assert(IsMainThread());
  uint8 mac[WG_COOKIE_LEN];
  const uint8 *data = packet->data;
  size_t data_size = packet->size;
  blake2s(mac, sizeof(mac), data, data_size - WG_COOKIE_LEN * 2, precomputed_mac1_key_, sizeof(precomputed_mac1_key_));
  return !memcmp_crypto(mac, data + data_size - WG_COOKIE_LEN * 2, WG_COOKIE_LEN);
}

void WgDevice::MakeCookie(uint8 cookie[WG_COOKIE_LEN], Packet *packet) {
  assert(IsMainThread());
  blake2s_state b2s;
  uint64 now = OsGetMilliseconds();
  if (now - cookie_secret_timestamp_ >= COOKIE_SECRET_MAX_AGE_MS) {
    cookie_secret_timestamp_ = now;
    OsGetRandomBytes(cookie_secret_, sizeof(cookie_secret_));
  }
  blake2s_init_key(&b2s, WG_COOKIE_LEN, cookie_secret_, sizeof(cookie_secret_));
  if (packet->addr.sin.sin_family == AF_INET)
    blake2s_update(&b2s, &packet->addr.sin.sin_addr, 4);
  else if (packet->addr.sin.sin_family == AF_INET6)
    blake2s_update(&b2s, &packet->addr.sin6.sin6_addr, sizeof(packet->addr.sin6.sin6_addr));
  blake2s_update(&b2s, &packet->addr.sin6.sin6_port, 2);
  blake2s_final(&b2s, cookie, WG_COOKIE_LEN);
}

bool WgDevice::CheckCookieMac2(Packet *packet) {
  assert(IsMainThread());
  uint8 cookie[WG_COOKIE_LEN];
  uint8 mac[WG_COOKIE_LEN];
  MakeCookie(cookie, packet);
  blake2s(mac, sizeof(mac), packet->data, packet->size - WG_COOKIE_LEN, cookie, sizeof(cookie));
  return !memcmp_crypto(mac, packet->data + packet->size - WG_COOKIE_LEN, WG_COOKIE_LEN);
}

void WgDevice::CreateCookieMessage(MessageHandshakeCookie *dst, Packet *packet, uint32 remote_key_id) {
  assert(IsMainThread());
  dst->type = MESSAGE_HANDSHAKE_COOKIE;
  dst->receiver_key_id = remote_key_id;
  MakeCookie(dst->cookie_enc, packet);
  OsGetRandomBytes(dst->nonce, sizeof(dst->nonce));
  MessageMacs *mac = (MessageMacs *)(packet->data + packet->size - sizeof(MessageMacs));
  xchacha20poly1305_encrypt(dst->cookie_enc, dst->cookie_enc, WG_COOKIE_LEN, mac->mac1, WG_COOKIE_LEN, dst->nonce, precomputed_cookie_key_);
}

void WgDevice::EraseKeypairAddrEntry_Locked(WgKeypair *kp) {
  // todo: figure out how to make this multithread safe.
  WgAddrEntry *ae = kp->addr_entry;

  assert(ae->ref_count >= 1);
  assert(ae->ref_count == !!ae->keys[0] + !!ae->keys[1] + !!ae->keys[2]);
  assert(ae->keys[kp->addr_entry_slot - 1] == kp);

  kp->addr_entry = NULL;

  ae->keys[kp->addr_entry_slot - 1] = NULL;
  kp->addr_entry_slot = 0;
  
  if (ae->ref_count-- == 1) {
    addr_entry_lookup_.erase(ae->addr_entry_id);
    delete ae;
  }
}

static WgAddrEntry::IpPort ConvertIpAddrToAddrX(const IpAddr &src) {
  WgAddrEntry::IpPort r;
  if (src.sin.sin_family == AF_INET) {
    Write64(r.bytes, src.sin.sin_addr.s_addr);
    Write64(r.bytes + 8, 0);
    Write32(r.bytes + 16, src.sin.sin_port);
  } else {
    memcpy(r.bytes, &src.sin6.sin6_addr, 16);
    Write32(r.bytes + 16, (AF_INET6 << 16) + src.sin6.sin6_port);
  }
  return r;
}

WgKeypair *WgDevice::LookupKeypairInAddrEntryMap(const IpAddr &addr, uint32 slot) {
  // Convert IpAddr to WgAddrEntry::IpPort suitable for use in hash.
  WgAddrEntry::IpPort addr_x = ConvertIpAddrToAddrX(addr);
  WG_SCOPED_RWLOCK_SHARED(addr_entry_lookup_lock_);
  auto it = addr_entry_lookup_.find(addr_x);
  if (it == addr_entry_lookup_.end())
    return NULL;
  WgAddrEntry *addr_entry = (WgAddrEntry*)it->second;
  return addr_entry->keys[slot];
}

void WgDevice::UpdateKeypairAddrEntry_Locked(const IpAddr &addr, WgKeypair *keypair) {
  assert(keypair->peer->IsPeerLocked());
  WgAddrEntry::IpPort addr_x = ConvertIpAddrToAddrX(addr);
  {
    WG_SCOPED_RWLOCK_SHARED(addr_entry_lookup_lock_);
    if (keypair->addr_entry != NULL && keypair->addr_entry->addr_entry_id == addr_x) {
      keypair->broadcast_short_key = 1;
      return;
    }
  }

  WG_SCOPED_RWLOCK_EXCLUSIVE(addr_entry_lookup_lock_);
  if (keypair->addr_entry != NULL)
    EraseKeypairAddrEntry_Locked(keypair);

  WgAddrEntry **aep = (WgAddrEntry**)&addr_entry_lookup_[addr_x], *ae;

  if ((ae = *aep) == NULL) {
    *aep = ae = new WgAddrEntry(addr_x);
  } else {
    // Ensure we don't insert new things in this addr entry too often.
    if (ae->time_of_last_insertion + 1000 * 60 > low_resolution_timestamp_)
      return;
  }

  ae->time_of_last_insertion = low_resolution_timestamp_;

  // Update slot #
  uint32 next_slot = ae->next_slot;
  ae->next_slot = (next_slot == 2) ? 0 : next_slot + 1;

  WgKeypair *old_keypair = ae->keys[next_slot];
  ae->keys[next_slot] = keypair;
  keypair->addr_entry = ae;
  keypair->addr_entry_slot = next_slot + 1;
  if (old_keypair != NULL) {
    old_keypair->addr_entry = NULL;
    old_keypair->addr_entry_slot = 0;
  } else {
    ae->ref_count++;
  }
  assert(ae->ref_count == !!ae->keys[0] + !!ae->keys[1] + !!ae->keys[2]);

  keypair->broadcast_short_key = 1;
}

WgPeer::WgPeer(WgDevice *dev) {
  assert(dev->IsMainThread());
  dev_ = dev;
  endpoint_.sin.sin_family = 0;
  data_endpoint_.sin.sin_family = 0;
  endpoint_protocol_ = 0;
  data_endpoint_protocol_ = 0;
  next_peer_ = NULL;
  peer_extra_data_ = NULL;
  curr_keypair_ = next_keypair_ = prev_keypair_ = NULL;
  expect_cookie_reply_ = false;
  has_mac2_cookie_ = false;
  pending_keepalive_ = false;
  marked_for_delete_ = false;
  allow_multicast_through_peer_ = false;
  allow_endpoint_change_ = true;
  local_key_id_during_hs_ = 0;
  last_handshake_init_timestamp_ = -1000000ll;
  last_handshake_init_recv_timestamp_ = 0;
  last_complete_handskake_timestamp_ = 0;
  persistent_keepalive_ms_ = 0;
  keepalive_timeout_ms_ = KEEPALIVE_TIMEOUT_MS;
  rx_bytes_ = 0;
  tx_bytes_ = 0;
  timers_ = 0;
  first_queued_packet_ = NULL;
  last_queued_packet_ptr_ = &first_queued_packet_;
  num_queued_packets_ = 0;
  handshake_attempts_ = 0;
  total_handshake_attempts_ = 0;
  num_ciphers_ = 0;
  cipher_prio_ = 0;
  main_thread_scheduled_ = 0;
  memset(last_timestamp_, 0, sizeof(last_timestamp_));
  ipv4_broadcast_addr_ = 0xffffffff;
  memset(features_, 0, sizeof(features_));
  memset(preshared_key_, 0, sizeof(preshared_key_));
  memset(&s_remote_, 0, sizeof(s_remote_));

  // Insert into the parent's linked list
  *dev_->last_peer_ptr_ = this;
  dev_->last_peer_ptr_ = &next_peer_;
}

WgPeer::~WgPeer() {
  // do not delete this directly, instead call RemovePeer
  assert(marked_for_delete_);
  assert(dev_->IsMainThread());
  assert(curr_keypair_ == NULL && next_keypair_ == NULL && prev_keypair_ == NULL);
  assert(local_key_id_during_hs_ == 0);
  assert(first_queued_packet_ == NULL);
  if (peer_extra_data_)
    peer_extra_data_->OnPeerDestroy();
}

void WgPeer::DelayedDelete(void *x) {
  WgPeer *peer = (WgPeer*)x;
  assert(peer->dev_->IsMainThread());

  if (peer->main_thread_scheduled_ != 0) {
    WG_ACQUIRE_LOCK(peer->dev_->main_thread_scheduled_lock_);
    // Unlink myself from the main thread scheduled list
    for (WgPeer **pp = &peer->dev_->main_thread_scheduled_; *pp; pp = &(*pp)->main_thread_scheduled_next_) {
      if (*pp == peer) {
        *pp = peer->main_thread_scheduled_next_;
        break;
      }
    }
    WG_RELEASE_LOCK(peer->dev_->main_thread_scheduled_lock_);
  }
  delete peer;
}

void WgPeer::RemovePeer() {
  assert(dev_->IsMainThread());
  assert(!marked_for_delete_);

  // Find and unlink the peer from the parent's peer list
  WgPeer **pp = &dev_->peers_;
  while (*pp != this)
    pp = &(*pp)->next_peer_;
  if ((*pp = next_peer_) == NULL)
    dev_->last_peer_ptr_ = pp;

  RemoveAllIps();
  dev_->peer_id_lookup_.erase(s_remote_);
  
  WG_ACQUIRE_LOCK(mutex_);
  marked_for_delete_ = true;
  ClearKeys_Locked();
  ClearHandshake_Locked();
  ClearPacketQueue_Locked();
  WG_RELEASE_LOCK(mutex_);

  // The WgPeer instance may still be accessible from
  // worker threads that already started processing a packet,
  // so defer the actual delete of it.
  dev_->delayed_delete_.Add(&WgPeer::DelayedDelete, this);
}

void WgPeer::ClearKeys_Locked() {
  assert(dev_->IsMainThread() && IsPeerLocked());
  DeleteKeypair(&curr_keypair_);
  DeleteKeypair(&next_keypair_);
  DeleteKeypair(&prev_keypair_);
}

void WgPeer::ClearHandshake_Locked() {
  assert(dev_->IsMainThread() && IsPeerLocked());
  uint32 v = local_key_id_during_hs_;
  if (v != 0) {
    local_key_id_during_hs_ = 0;
    WG_SCOPED_RWLOCK_EXCLUSIVE(dev_->key_id_lookup_lock_);
    dev_->key_id_lookup_.erase(v);
  }
}

void WgPeer::ClearPacketQueue_Locked() {
  assert(dev_->IsMainThread() && IsPeerLocked());
  Packet *packet;
  while ((packet = first_queued_packet_) != NULL) {
    first_queued_packet_ = Packet_NEXT(packet);
    FreePacket(packet);
  }
  last_queued_packet_ptr_ = &first_queued_packet_;
  num_queued_packets_ = 0;
}

void WgPeer::AddPacketToPeerQueue_Locked(Packet *packet) {
  assert(IsPeerLocked());
  assert(!marked_for_delete_);
  // Keep only the first MAX_QUEUED_PACKETS packets.
  while (num_queued_packets_ >= MAX_QUEUED_PACKETS_PER_PEER) {
    Packet *packet = first_queued_packet_;
    first_queued_packet_ = Packet_NEXT(packet);
    num_queued_packets_--;
    FreePacket(packet);
  }
  // Add the packet to the out queue that will get sent once handshake completes
  *last_queued_packet_ptr_ = packet;
  last_queued_packet_ptr_ = &Packet_NEXT(packet);
  Packet_NEXT(packet) = NULL;
  num_queued_packets_++;
}

void WgPeer::SetPublicKey(const WgPublicKey &spub) {
  assert(dev_->IsMainThread());
  assert(IsOnlyZeros(s_remote_.bytes, sizeof(s_remote_.bytes)) || 
         memcmp(s_remote_.bytes, spub.bytes, sizeof(s_remote_.bytes)) == 0);

  s_remote_ = spub;
  dev_->peer_id_lookup_[s_remote_] = this;

  if (!dev_->is_private_key_initialized_)
    return;

  // Precompute: s_priv_pub_ := DH(sprivr, spubi)
  curve25519_donna(s_priv_pub_, dev_->s_priv_, s_remote_.bytes);
  // Precompute: precomputed_cookie_key_ := HASH(LABEL-COOKIE || Spub_m)
  //             precomputed_mac1_key_   := HASH(MAC1-COOKIE || Spub_m)
  BlakeX2(precomputed_cookie_key_, sizeof(precomputed_cookie_key_),
          kLabelCookie, sizeof(kLabelCookie), s_remote_.bytes, WG_PUBLIC_KEY_LEN);
  BlakeX2(precomputed_mac1_key_, sizeof(precomputed_mac1_key_),
          kLabelMac1, sizeof(kLabelMac1), s_remote_.bytes, WG_PUBLIC_KEY_LEN);

  // Remove the peer's keys
  WG_ACQUIRE_LOCK(mutex_);
  ClearKeys_Locked();
  ClearHandshake_Locked();
  WG_RELEASE_LOCK(mutex_);
}

void WgPeer::SetPresharedKey(const uint8 preshared_key[WG_SYMMETRIC_KEY_LEN]) {
  if (preshared_key)
    memcpy(preshared_key_, preshared_key, sizeof(preshared_key_));
  else
    memset(preshared_key_, 0, sizeof(preshared_key_));
}

// run on the client
void WgPeer::CreateMessageHandshakeInitiation(Packet *packet) {
  assert(dev_->IsMainThread());

  uint8 k[WG_SYMMETRIC_KEY_LEN];
  MessageHandshakeInitiation *dst = (MessageHandshakeInitiation *)packet->data;

  // Ci := HASH(CONSTRUCTION)
  memcpy(hs_.ci, kWgInitChainingKey, sizeof(hs_.ci));
  // Hi := HASH(Ci || IDENTIFIER)
  memcpy(hs_.hi, kWgInitHash, sizeof(hs_.hi));
  // Hi := HASH(Hi || Spub_r)
  BlakeMix(hs_.hi, s_remote_.bytes, sizeof(s_remote_));
  // (Epriv_r, Epub_r) := DH-GENERATE()
  // msg.ephemeral = Epub_r
  OsGetRandomBytes(hs_.e_priv, sizeof(hs_.e_priv));
  curve25519_normalize(hs_.e_priv);
  curve25519_donna(dst->ephemeral, hs_.e_priv, kCurve25519Basepoint);
  // Ci := KDF_1(Ci, msg.ephemeral)
  blake2s_hkdf(hs_.ci, sizeof(hs_.ci), NULL, 32, NULL, 32, dst->ephemeral, sizeof(dst->ephemeral), hs_.ci, WG_HASH_LEN);
  // Hi := HASH(Hi || msg.ephemeral)
  BlakeMix(hs_.hi, dst->ephemeral, sizeof(dst->ephemeral));
  // (Ci, K) := KDF2(Ci, DH(epriv, spub_r))
  ComputeHKDF2DH(hs_.ci, k, hs_.e_priv, s_remote_.bytes);
  // msg.static = AEAD(K, 0, Spub_i, Hi)
  chacha20poly1305_encrypt(dst->static_enc, dev_->s_pub_, sizeof(dev_->s_pub_), hs_.hi, sizeof(hs_.hi), 0, k);
  // Hi := HASH(Hi || msg.static)
  BlakeMix(hs_.hi, dst->static_enc, sizeof(dst->static_enc));
  // (Ci, K) := KDF2(Ci, DH(sprivr, spubi))
  blake2s_hkdf(hs_.ci, sizeof(hs_.ci), k, sizeof(k), NULL, 32, s_priv_pub_, sizeof(s_priv_pub_), hs_.ci, WG_HASH_LEN);
  // TAI64N
  OsGetTimestampTAI64N(dst->timestamp_enc);


  int extfield_size = 0;
  if (WITH_HANDSHAKE_EXT) {
    extfield_size = WriteHandshakeExtension(dst->timestamp_enc + WG_TIMESTAMP_LEN + 4, NULL);
    if (dev_->plugin_) {
      uint32 rv = dev_->plugin_->OnHandshake0(this, dst->timestamp_enc + WG_TIMESTAMP_LEN + 4 + extfield_size, MAX_SIZE_OF_HANDSHAKE_EXTENSION - 4 - extfield_size, dst->ephemeral);
      assert(!(rv & WgPlugin::kHandshakeResponseFail));
      extfield_size += rv;
    }
    if (extfield_size) {
      WriteLE32(dst->timestamp_enc + WG_TIMESTAMP_LEN, kTunsafeExtensionClientID);
      extfield_size += 4;
    }
  }
  // msg.timestamp := AEAD(K, 0, timestamp, hi)
  chacha20poly1305_encrypt(dst->timestamp_enc, dst->timestamp_enc, extfield_size + WG_TIMESTAMP_LEN, hs_.hi, sizeof(hs_.hi), 0, k);
  // Hi := HASH(Hi || msg.timestamp)
  BlakeMix(hs_.hi, dst->timestamp_enc, extfield_size + WG_TIMESTAMP_LEN + WG_MAC_LEN);

  packet->size = (unsigned)(sizeof(MessageHandshakeInitiation) + extfield_size);

  dst->sender_key_id = dev_->InsertInKeyIdLookup(this, NULL);
  dst->type = MESSAGE_HANDSHAKE_INITIATION;
  memzero_crypto(k, sizeof(k));
  WriteMacToPacket((uint8*)dst, (MessageMacs*)((uint8*)&dst->mac + extfield_size));
}

// Parsed by server
WgPeer *WgPeer::ParseMessageHandshakeInitiation(WgDevice *dev, Packet *packet) { // const MessageHandshakeInitiation *src, MessageHandshakeResponse *dst) {
  assert(dev->IsMainThread());
  // Copy values into handshake once we've validated it all.
  uint8 ci[WG_HASH_LEN];
  uint8 hi[WG_HASH_LEN];
  union {
    uint8 k[WG_SYMMETRIC_KEY_LEN];
    uint8 e_priv[WG_PUBLIC_KEY_LEN];
  };
  union {
    WgPublicKey spubi;
    uint8 e_remote[WG_PUBLIC_KEY_LEN];
    uint8 hi2[WG_HASH_LEN];
  };
  uint8 t[WG_HASH_LEN];
  WgPeer *peer;
  WgKeypair *keypair = NULL;
  uint32 remote_key_id;
  uint64 now;
  uint8 extbuf[MAX_SIZE_OF_HANDSHAKE_EXTENSION + WG_TIMESTAMP_LEN];
  MessageHandshakeInitiation *src = (MessageHandshakeInitiation *)packet->data;
  MessageHandshakeResponse *dst;
  int extfield_size;

  // Ci := HASH(CONSTRUCTION)
  memcpy(ci, kWgInitChainingKey, sizeof(ci));
  // Hi := HASH(Ci || IDENTIFIER)
  memcpy(hi, kWgInitHash, sizeof(hi));
  // Hi := HASH(Hi || Spub_r)
  BlakeMix(hi, dev->s_pub_, sizeof(dev->s_pub_));
  // Ci := KDF_1(Ci, msg.ephemeral)
  blake2s_hkdf(ci, sizeof(ci), NULL, 32, NULL, 32, src->ephemeral, sizeof(src->ephemeral), ci, WG_HASH_LEN);
  // Hi := HASH(Hi || msg.ephemeral)
  BlakeMix(hi, src->ephemeral, sizeof(src->ephemeral));
  // (Ci, K) := KDF2(Ci, DH(spriv, msg.ephemeral))
  ComputeHKDF2DH(ci, k, dev->s_priv_, src->ephemeral);
  // Spub_i = AEAD_DEC(K, 0, msg.static, Hi)
  if (!chacha20poly1305_decrypt(spubi.bytes, src->static_enc, sizeof(src->static_enc), hi, sizeof(hi), 0, k))
    goto getout;
  // Hi := HASH(Hi || msg.static)
  BlakeMix(hi, src->static_enc, sizeof(src->static_enc));
  // Lookup the peer with this ID
  while ((peer = dev->GetPeerFromPublicKey(spubi)) == NULL) {
    if (dev->plugin_ == NULL || !dev->plugin_->HandleUnknownPeerId(spubi.bytes, packet))
      goto getout;
  }
  // (Ci, K) := KDF2(Ci, DH(sprivr, spubi))
  blake2s_hkdf(ci, sizeof(ci), k, sizeof(k), NULL, 32, peer->s_priv_pub_, sizeof(peer->s_priv_pub_), ci, WG_HASH_LEN);
  // Hi2 := Hi
  memcpy(hi2, hi, sizeof(hi2));
  extfield_size = packet->size - sizeof(MessageHandshakeInitiation);
  if ((uint32)extfield_size > MAX_SIZE_OF_HANDSHAKE_EXTENSION)
    goto getout;
  // Hi := HASH(Hi || msg.timestamp)
  BlakeMix(hi, src->timestamp_enc, extfield_size + WG_TIMESTAMP_LEN + WG_MAC_LEN);
  // TIMESTAMP := AEAD_DEC(K, 0, msg.timestamp, hi2)
  if (!chacha20poly1305_decrypt(extbuf, src->timestamp_enc, extfield_size + WG_TIMESTAMP_LEN + WG_MAC_LEN, hi2, sizeof(hi2), 0, k))
    goto getout;
  // Replay attack?
  if (memcmp(extbuf, peer->last_timestamp_, WG_TIMESTAMP_LEN) <= 0)
    goto getout;
  // Flood attack?
  now = OsGetMilliseconds();
  if (now < peer->last_handshake_init_recv_timestamp_ + MIN_HANDSHAKE_INTERVAL_MS)
    goto getout;

  // Remember all the information we need to produce a response cause we cannot touch src again
  peer->last_handshake_init_recv_timestamp_ = now;
  memcpy(peer->last_timestamp_, extbuf, sizeof(peer->last_timestamp_));
  
  memcpy(e_remote, src->ephemeral, sizeof(e_remote));
  remote_key_id = src->sender_key_id;

  dst = (MessageHandshakeResponse *)src;
  dst->receiver_key_id = remote_key_id;
  // (Epriv_r, Epub_r) := DH-GENERATE()
  // msg.ephemeral = Epub_r
  OsGetRandomBytes(e_priv, sizeof(e_priv));
  curve25519_normalize(e_priv);
  curve25519_donna(dst->ephemeral, e_priv, kCurve25519Basepoint);
  // Hr := HASH(Hr || msg.ephemeral)
  BlakeMix(hi, dst->ephemeral, sizeof(dst->ephemeral));
  // Ci := KDF_1(Ci, msg.ephemeral)
  blake2s_hkdf(ci, sizeof(ci), NULL, 32, NULL, 32, dst->ephemeral, sizeof(dst->ephemeral), ci, WG_HASH_LEN);
  // Ci : = KDF2(Ci, DH(epriv, epub))
  ComputeHKDF2DH(ci, NULL, e_priv, e_remote);
  // Ci : = KDF2(Ci, DH(epriv, spub))
  ComputeHKDF2DH(ci, NULL, e_priv, peer->s_remote_.bytes);
  // (Ci, T, K) := KDF3(Ci, Q)
  blake2s_hkdf(ci, sizeof(ci), t, sizeof(t), k, sizeof(k), peer->preshared_key_, sizeof(preshared_key_), ci, WG_HASH_LEN);
  // Hr := HASH(Hr || T)
  BlakeMix(hi, t, sizeof(t));

  keypair = WgPeer::CreateNewKeypair(false, ci, remote_key_id);
  if (keypair) {
    if (WITH_HANDSHAKE_EXT && !peer->ParseExtendedHandshake(keypair, extbuf + WG_TIMESTAMP_LEN, extfield_size))
      goto getout;

    int extfield_out_size = 0;
    if (WITH_HANDSHAKE_EXT && extfield_size)
      extfield_out_size = peer->WriteHandshakeExtension(dst->empty_enc + 4, keypair);

    // Allow plugin to determine what to do with the packet,
    // it can append new headers to the response, and decide what to do.  
    if (WITH_HANDSHAKE_EXT && dev->plugin_) {
      uint32 rv = dev->plugin_->OnHandshake1(peer, extbuf + WG_TIMESTAMP_LEN, extfield_size, e_remote,
                                                     dst->empty_enc + 4 + extfield_out_size, MAX_SIZE_OF_HANDSHAKE_EXTENSION - 4 - extfield_out_size, dst->ephemeral);
      if (rv == WgPlugin::kHandshakeResponseDrop)
        goto getout;
      if (rv & WgPlugin::kHandshakeResponseFail)
        delete exch_null(keypair);
      extfield_out_size += rv & ~WgPlugin::kHandshakeResponseFail;
    }
    if (extfield_out_size) {
      WriteLE32(dst->empty_enc, kTunsafeExtensionClientID);
      extfield_out_size += 4;
    }
    
    dst->sender_key_id = keypair ? dev->InsertInKeyIdLookup(peer, keypair) : 0;

    WG_ACQUIRE_LOCK(peer->mutex_);
    peer->rx_bytes_ += packet->size;
    if (keypair != NULL) {
      // The server side needs to remember the endpoint on incoming handshakes.
      if (peer->allow_endpoint_change_ && keypair->enabled_features[WG_FEATURE_HYBRID_TCP]) {
        peer->endpoint_ = packet->addr;
        peer->endpoint_protocol_ = packet->protocol;
      }
      peer->InsertKeypairInPeer_Locked(keypair);
      peer->OnHandshakeAuthComplete();
    }
    packet->size = (unsigned)(sizeof(MessageHandshakeResponse) + extfield_out_size);
    peer->tx_bytes_ += packet->size;
    WG_RELEASE_LOCK(peer->mutex_);


    // msg.empty := AEAD(K, 0, "", Hr)
    chacha20poly1305_encrypt(dst->empty_enc, dst->empty_enc, extfield_out_size, hi, sizeof(hi), 0, k);
    // Hr := HASH(Hr || "")
    //BlakeMix(hi, dst->empty_enc, extfield_out_size);
    
    dst->type = MESSAGE_HANDSHAKE_RESPONSE;
    peer->WriteMacToPacket((uint8*)dst, (MessageMacs*)((uint8*)&dst->mac + extfield_out_size));
  } else {
getout:
    delete keypair;
    peer = NULL;
  }
  memzero_crypto(hi, sizeof(hi));
  memzero_crypto(ci, sizeof(ci));
  memzero_crypto(k, sizeof(k));
  memzero_crypto(t, sizeof(t));
  return peer;
}

WgPeer *WgPeer::ParseMessageHandshakeResponse(WgDevice *dev, const Packet *packet) {
  assert(dev->IsMainThread());
  MessageHandshakeResponse *src = (MessageHandshakeResponse *)packet->data;
  uint8 t[WG_HASH_LEN];
  uint8 k[WG_SYMMETRIC_KEY_LEN];
  WgKeypair *keypair;
  auto peer_and_keypair = dev->LookupPeerInKeyIdLookup(src->receiver_key_id);
  if (peer_and_keypair == NULL)
    return NULL;
  WgPeer *peer = peer_and_keypair->first;
  assert(src->receiver_key_id == peer->local_key_id_during_hs_);

  HandshakeState hs = peer->hs_;
  // Hr := HASH(Hr || msg.ephemeral)
  BlakeMix(hs.hi, src->ephemeral, sizeof(src->ephemeral));
  // Ci := KDF_1(Ci, msg.ephemeral)
  blake2s_hkdf(hs.ci, sizeof(hs.ci), NULL, 32, NULL, 32, src->ephemeral, sizeof(src->ephemeral), hs.ci, sizeof(hs.ci));
  // Ci : = KDF2(Ci, DH(epriv, epub))
  ComputeHKDF2DH(hs.ci, NULL, hs.e_priv, src->ephemeral);
  // Ci : = KDF2(Ci, DH(spriv, epub))
  ComputeHKDF2DH(hs.ci, NULL, peer->dev_->s_priv_, src->ephemeral);
  // (Ci, T, K) := KDF3(Ci, Q)
  blake2s_hkdf(hs.ci, sizeof(hs.ci), t, sizeof(t), k, sizeof(k), peer->preshared_key_, sizeof(peer->preshared_key_), hs.ci, sizeof(hs.ci));
  // Hr := HASH(Hr || T)
  BlakeMix(hs.hi, t, sizeof(t));

  int extfield_size = packet->size - sizeof(MessageHandshakeResponse);
  if ((uint32)extfield_size > MAX_SIZE_OF_HANDSHAKE_EXTENSION)
    goto getout;

  // "" := AEAD_DEC(K, 0, msg.empty, Hr)
  if (!chacha20poly1305_decrypt(src->empty_enc, src->empty_enc, extfield_size + sizeof(src->empty_enc), hs.hi, sizeof(hs.hi), 0, k))
    goto getout;

  keypair = WgPeer::CreateNewKeypair(true, hs.ci, src->sender_key_id);
  if (!keypair)
    goto getout;

  if (WITH_HANDSHAKE_EXT && !peer->ParseExtendedHandshake(keypair, src->empty_enc, extfield_size)) {
    delete keypair;
    goto getout;
  }

  // Allow plugin to determine what to do with the packet,
  // it can append new headers to the response, and decide what to do.  
  if (WITH_HANDSHAKE_EXT && dev->plugin_) {
    uint32 rv = dev->plugin_->OnHandshake2(peer, src->empty_enc, extfield_size, src->ephemeral);
    if (rv & WgPlugin::kHandshakeResponseFail) {
      delete keypair;
      goto getout;
    }
  }

  // Re-map the entry in the id table so it points at this keypair instead.
  keypair->local_key_id = peer->local_key_id_during_hs_;
  peer->local_key_id_during_hs_ = 0;
  peer_and_keypair->second = keypair;

  WG_ACQUIRE_LOCK(peer->mutex_);
  if (peer->allow_endpoint_change_) {
    // TODO: Why is this needed, if we are able to get a response for the handshake init
    // packet then we already know its endpoint?
    peer->endpoint_protocol_ = packet->protocol;
    peer->endpoint_ = packet->addr;
    if (!keypair->enabled_features[WG_FEATURE_HYBRID_TCP] || !peer->IsTransientDataEndpointActive()) {
      peer->data_endpoint_protocol_ = peer->endpoint_protocol_;
      peer->data_endpoint_ = peer->endpoint_;
    }
  // If hybrid tcp mode was enabled for the connection, switch
  // the data endpoint to the udp endpoint.
  } else if (peer->endpoint_protocol_ == kPacketProtocolTcp) {
    peer->data_endpoint_protocol_ = keypair->enabled_features[WG_FEATURE_HYBRID_TCP] ? kPacketProtocolUdp : kPacketProtocolTcp;
    peer->data_endpoint_ = peer->endpoint_;
  }

  WG_EXTENSION_HOOKS::OnPeerIncomingUdp(peer, packet, packet->size);
  
  peer->rx_bytes_ += packet->size;
  peer->InsertKeypairInPeer_Locked(keypair);
  WG_RELEASE_LOCK(peer->mutex_);

  if (0) {
getout:
    peer = NULL;
  }
  memzero_crypto(t, sizeof(t));
  memzero_crypto(k, sizeof(k));
  memzero_crypto(&hs, sizeof(hs));
 
  return peer;
}

// This is parsed by the initiator, when it needs to re-send the handshake message with a better mac.
void WgPeer::ParseMessageHandshakeCookie(WgDevice *dev, const MessageHandshakeCookie *src) {
  assert(dev->IsMainThread());
  uint8 cookie[WG_COOKIE_LEN];
  auto peer_and_keypair = dev->LookupPeerInKeyIdLookup(src->receiver_key_id);
  if (!peer_and_keypair)
    return;
  WgPeer *peer = peer_and_keypair->first;
  if (!peer->expect_cookie_reply_)
    return;
  if (!xchacha20poly1305_decrypt(cookie, src->cookie_enc, sizeof(src->cookie_enc), 
                                 peer->sent_mac1_, sizeof(peer->sent_mac1_), src->nonce, peer->precomputed_cookie_key_))
    return;
  WG_ACQUIRE_LOCK(peer->mutex_);
  peer->rx_bytes_ += sizeof(MessageHandshakeCookie);
  WG_RELEASE_LOCK(peer->mutex_);
  peer->expect_cookie_reply_ = false;
  peer->has_mac2_cookie_ = true;
  peer->mac2_cookie_timestamp_ = OsGetMilliseconds();
  memcpy(peer->mac2_cookie_, cookie, sizeof(peer->mac2_cookie_));
}

int WgPeer::WriteHandshakeExtension(uint8 *dst, WgKeypair *keypair) {
  uint8 *dst_org = dst, *dst_end = dst + MAX_SIZE_OF_HANDSHAKE_EXTENSION;

  if (WITH_HANDSHAKE_EXT) {
    if (WITH_BOOLEAN_FEATURES) {
      uint8 value = 0;
      // Include the supported features extension
      if (!IsOnlyZeros(features_, sizeof(features_))) {
        *dst++ = kExtensionType_Booleans;
        *dst++ = (WG_FEATURES_COUNT + 3) >> 2;
        for (size_t i = 0; i != WG_FEATURES_COUNT; i++) {
          if ((i & 3) == 0)
            value = 0;
          dst[i >> 2] = (value += (features_[i] << ((i * 2) & 7)));
        }
        // swap WG_FEATURE_ID_SKIP_KEYID_IN and WG_FEATURE_ID_SKIP_KEYID_OUT
        dst[1] = (dst[1] & 0xF0) + ((dst[1] >> 2) & 0x03) + ((dst[1] << 2) & 0x0C);
        dst += (WG_FEATURES_COUNT + 3) >> 2;
      }
    }
    if (WITH_CIPHER_SUITES) {
      // Ordered list of cipher suites
      size_t ciphers = num_ciphers_;
      if (ciphers) {
        *dst++ = kExtensionType_CipherSuites + cipher_prio_;
        if (keypair) {
          *dst++ = 1;
          *dst++ = keypair->cipher_suite;
        } else {
          *dst++ = (uint8)ciphers;
          memcpy(dst, ciphers_, ciphers);
          dst += ciphers;
        }
      }
    }
  }
  return (int)(dst - dst_org);
}

static bool ResolveBooleanFeatureValue(uint8 other, uint8 self, bool *result) {
  uint8 both = other * 4 + self;
  *result = (0xfec0 >> both) & 1;
  return (0xeff7 >> both) & 1;
}

static const uint8 cipher_strengths[EXT_CIPHER_SUITE_COUNT] = {4,2,3,1};

static uint32 ResolveCipherSuite(int tie, const uint8 *a, size_t a_size, const uint8 *b, size_t b_size) {
  uint32 abits[8] = {0}, bbits[8] = {0}, found_a = 0, found_b = 0;
  for (size_t i = 0; i < a_size; i++)
    abits[a[i] >> 5] |= 1 << (a[i] & 31);
  for (size_t i = 0; i < b_size; i++)
    bbits[b[i] >> 5] |= 1 << (b[i] & 31);
  for (size_t i = 0; i < a_size; i++)
    if (bbits[a[i] >> 5] & (1 << (a[i] & 31))) {
      found_a = a[i];
      break;
    }
  for (size_t i = 0; i < b_size; i++)
    if (abits[b[i] >> 5] & (1 << (b[i] & 31))) {
      found_b = b[i];
      break;
    }
  return (tie > 0 ||
          (tie == 0 && cipher_strengths[found_a] > cipher_strengths[found_b])) ? found_a : found_b;
}

static void WgKeypairDelayedDelete(void *x) {
  WgKeypair *t = (WgKeypair*)x;
  if (t->aes_gcm128_context_)
    free(t->aes_gcm128_context_);
  delete t;
}

void WgPeer::DeleteKeypair(WgKeypair **kp) {
  WgKeypair *t = *kp;
  *kp = NULL;
  if (t) {
    assert(t->peer->IsPeerLocked());
    WgDevice *dev = t->peer->dev_;
    if (t->addr_entry) {
      WG_SCOPED_RWLOCK_EXCLUSIVE(dev->addr_entry_lookup_lock_);
      dev->EraseKeypairAddrEntry_Locked(t);
    }
    if (t->local_key_id) {
      WG_SCOPED_RWLOCK_EXCLUSIVE(dev->key_id_lookup_lock_);
      dev->key_id_lookup_.erase(t->local_key_id);
      t->local_key_id = 0;
    }
    t->recv_key_state = WgKeypair::KEY_INVALID;
    dev->delayed_delete_.Add(&WgKeypairDelayedDelete, t);
  }
}

bool WgPeer::ParseExtendedHandshake(WgKeypair *kp, const uint8 *data, size_t data_size) {
  // Empty handshake is always OK
  if (data_size == 0)
    return true;

  // The first four bytes contain a client ID and major, minor version
  if (data_size < 4 || (ReadLE32(data) & 0xFFFFFF) != kTunsafeExtensionClientID)
    return false;
  data += 4, data_size -= 4;

  while (data_size >= 2) {
    uint8 type = data[0], size = data[1];
    data += 2, data_size -= 2;
    if (size > data_size)
      return false;
    switch (type) {
    case kExtensionType_CipherSuitesPrio:
    case kExtensionType_CipherSuites:
      if (WITH_CIPHER_SUITES) {
        kp->cipher_suite = ResolveCipherSuite(cipher_prio_ - (type - kExtensionType_CipherSuites),
                                              ciphers_, num_ciphers_, data, size);
      }
      break;

    case kExtensionType_Booleans:
      if (WITH_BOOLEAN_FEATURES) {
        for (uint32 i = 0, j = std::max<uint32>(WG_FEATURES_COUNT, size * 4); i != j; i++) {
          uint8 value = (i < (uint32)size * 4) ? (data[i >> 2] >> ((i * 2) & 7)) & 3 : 0;
          if (i >= WG_FEATURES_COUNT ? (value == WG_BOOLEAN_FEATURE_ENFORCES) :
              !ResolveBooleanFeatureValue(value, features_[i], &kp->enabled_features[i]))
            return false;
        }
      }
      break;
    }
    data += size, data_size -= size;
  }
  if (data_size != 0)
    return false;

  if (WITH_BOOLEAN_FEATURES && WITH_SHORT_MAC)
    kp->auth_tag_length = (kp->enabled_features[WG_FEATURE_ID_SHORT_MAC] ? 8 : CHACHA20POLY1305_AUTHTAGLEN);

  if (WITH_CIPHER_SUITES && kp->cipher_suite >= EXT_CIPHER_SUITE_AES128_GCM && kp->cipher_suite <= EXT_CIPHER_SUITE_AES256_GCM) {
#if WITH_AESGCM
    kp->aes_gcm128_context_ = (AesGcm128StaticContext *)malloc(sizeof(*kp->aes_gcm128_context_) * 2);
    if (!kp->aes_gcm128_context_)
      return false;
    int key_size = (kp->cipher_suite == EXT_CIPHER_SUITE_AES128_GCM) ? 128 : 256;
    CRYPTO_gcm128_init(&kp->aes_gcm128_context_[0], kp->send_key, key_size);
    CRYPTO_gcm128_init(&kp->aes_gcm128_context_[1], kp->recv_key, key_size);
#else  // WITH_AESGCM
    return false;
#endif  // WITH_AESGCM
  }

  return true;
}

WgKeypair *WgPeer::CreateNewKeypair(bool is_initiator, const uint8 chaining_key[WG_HASH_LEN], uint32 remote_key_id) {
  WgKeypair *kp = new WgKeypair;
  uint8 *first_key, *second_key;
  if (!kp)
    return NULL;
  memset(kp, 0, offsetof(WgKeypair, replay_detector));
  kp->is_initiator = is_initiator;
  kp->remote_key_id = remote_key_id;
  kp->auth_tag_length = CHACHA20POLY1305_AUTHTAGLEN;
  
  first_key = kp->send_key, second_key = kp->recv_key;
  if (!is_initiator)
    std::swap(first_key, second_key);
  blake2s_hkdf(first_key, sizeof(kp->send_key), second_key, sizeof(kp->recv_key), 
               kp->auth_tag_length != CHACHA20POLY1305_AUTHTAGLEN ? (uint8*)kp->compress_mac_keys : NULL, 32, 
               NULL, 0, chaining_key, WG_HASH_LEN);

  if (!is_initiator) {
    std::swap(kp->compress_mac_keys[0][0], kp->compress_mac_keys[1][0]);
    std::swap(kp->compress_mac_keys[0][1], kp->compress_mac_keys[1][1]);
  }

  kp->send_key_state = kp->recv_key_state = WgKeypair::KEY_VALID;
  kp->key_timestamp = OsGetMilliseconds();
  return kp;
}

void WgPeer::InsertKeypairInPeer_Locked(WgKeypair *kp) {
  assert(dev_->IsMainThread() && IsPeerLocked());
  assert(kp->peer == NULL);
  kp->peer = this;
  time_of_next_key_event_ = 0;
  DeleteKeypair(&prev_keypair_);
  if (kp->is_initiator) {
    // When we're the initator then we got the handshake and we can
    // use the keypair right away.
    if (next_keypair_) {
      prev_keypair_ = next_keypair_;
      next_keypair_ = NULL;
      DeleteKeypair(&curr_keypair_);
    } else {
      prev_keypair_ = curr_keypair_;
    }
    curr_keypair_ = kp;
  } else {
    // The keypair will be moved to curr when we get the first data packet.
    DeleteKeypair(&next_keypair_);
    next_keypair_ = kp;
  }
}

bool WgPeer::CheckSwitchToNextKey_Locked(WgKeypair *keypair) {
  assert(IsPeerLocked());
  if (keypair != next_keypair_)
    return false;
  DeleteKeypair(&prev_keypair_);
  prev_keypair_ = curr_keypair_;
  curr_keypair_ = next_keypair_;
  next_keypair_ = NULL;
  time_of_next_key_event_ = 0;
  return true;
}

bool WgPeer::CheckHandshakeRateLimit() {
  assert(dev_->IsMainThread());
  uint64 now = OsGetMilliseconds();
  if (now - last_handshake_init_timestamp_ < REKEY_TIMEOUT_MS)
    return false;
  last_handshake_init_timestamp_ = now;
  return true;
}

void WgPeer::WriteMacToPacket(const uint8 *data, MessageMacs *dst) {
  assert(dev_->IsMainThread());
  expect_cookie_reply_ = true;
  blake2s(dst->mac1, sizeof(dst->mac1), data, (uint8*)dst->mac1 - data, precomputed_mac1_key_, sizeof(precomputed_mac1_key_));
  memcpy(sent_mac1_, dst->mac1, sizeof(sent_mac1_));
  if (has_mac2_cookie_ && OsGetMilliseconds() - mac2_cookie_timestamp_ < COOKIE_SECRET_MAX_AGE_MS - COOKIE_SECRET_LATENCY_MS) {
    blake2s(dst->mac2, sizeof(dst->mac2), data, (uint8*)dst->mac2 - data, mac2_cookie_,  sizeof(mac2_cookie_));
  } else {
    has_mac2_cookie_ = false;

    if (dev_->packet_obfuscator().enabled()) {
      // when obfuscation is enabled just make the top bits random
      for (size_t i = 0; i < 4; i++)
        ((uint32*)dst->mac2)[i] = dev_->GetRandomNumber();
    } else {
      memset(dst->mac2, 0, sizeof(dst->mac2));
    }
  }
}

enum {
  // Timer for retransmitting the handshake if we don't hear back after REKEY_TIMEOUT_MS
  TIMER_RETRANSMIT_HANDSHAKE = 0,
  // Timer for sending keepalive if we received a packet if we don't send anything else for KEEPALIVE_TIMEOUT_MS
  TIMER_SEND_KEEPALIVE = 1,
  // Timer for initiating new handshake if we have sent a packet but after have not received one for KEEPALIVE_TIMEOUT_MS + REKEY_TIMEOUT_MS
  TIMER_NEW_HANDSHAKE = 2,
  // Timer for zeroing out all keys and handshake state after (REJECT_AFTER_TIME_MS * 3) if no new keys have been received
  TIMER_ZERO_KEYS = 3,
  // Timer for sending a keepalive packet every PERSISTENT_KEEPALIVE_MS
  TIMER_PERSISTENT_KEEPALIVE = 4,
  // Timer for removing the transient UDP endpoint in hybrid TCP mode after 10 seconds
  TIMER_HYBRID_TCP = 5,

  TIMERS_COUNT = 6,
};

#define WgClearTimer(x) (timers_ &= ~(((1<<TIMERS_COUNT)+1) << x))
#define WgIsTimerActive(x) (timers_ & (((1<<TIMERS_COUNT)+1) << x))
#define WgSetTimer(x) (timers_ |= (((1<<TIMERS_COUNT)) << (x)))

void WgPeer::OnDataSent() {
  assert(IsPeerLocked());
  WgClearTimer(TIMER_SEND_KEEPALIVE);
  if (!WgIsTimerActive(TIMER_NEW_HANDSHAKE))
    WgSetTimer(TIMER_NEW_HANDSHAKE);
  WgSetTimer(TIMER_PERSISTENT_KEEPALIVE);
}

void WgPeer::OnKeepaliveSent() {
  assert(IsPeerLocked());
  WgSetTimer(TIMER_PERSISTENT_KEEPALIVE);
}

void WgPeer::OnDataReceived() {
  assert(IsPeerLocked());
  WgClearTimer(TIMER_NEW_HANDSHAKE);
  if (!WgIsTimerActive(TIMER_SEND_KEEPALIVE))
    WgSetTimer(TIMER_SEND_KEEPALIVE);
  else
    pending_keepalive_ = true;
  WgSetTimer(TIMER_PERSISTENT_KEEPALIVE);
  WgSetTimer(TIMER_HYBRID_TCP);
}

void WgPeer::OnKeepaliveReceived() {
  assert(IsPeerLocked());
  WgClearTimer(TIMER_NEW_HANDSHAKE);
  WgSetTimer(TIMER_PERSISTENT_KEEPALIVE);
  WgSetTimer(TIMER_HYBRID_TCP);
}

void WgPeer::OnHandshakeInitSent() {
  assert(IsPeerLocked());
  WgClearTimer(TIMER_SEND_KEEPALIVE);
  WgSetTimer(TIMER_RETRANSMIT_HANDSHAKE);
  WgSetTimer(TIMER_PERSISTENT_KEEPALIVE);
}

void WgPeer::OnHandshakeAuthComplete() {
  assert(IsPeerLocked());
  WgClearTimer(TIMER_NEW_HANDSHAKE);
  WgSetTimer(TIMER_ZERO_KEYS);
  WgSetTimer(TIMER_PERSISTENT_KEEPALIVE);
}

static const char * const kCipherSuites[] = {
  "chacha20-poly1305",
  "aes128-gcm",
  "aes256-gcm",
  "none"
};

void WgPeer::OnHandshakeFullyComplete() {
  assert(IsPeerLocked());
  WgClearTimer(TIMER_RETRANSMIT_HANDSHAKE);
  total_handshake_attempts_ = handshake_attempts_ = 0;

  uint64 now = OsGetMilliseconds();

  if (last_complete_handskake_timestamp_ == 0) {
    bool any_feature = false;
    for(size_t i = 0; i < WG_FEATURES_COUNT; i++)
      any_feature |= curr_keypair_->enabled_features[i];
    if (curr_keypair_->cipher_suite != 0 || any_feature) {
      RINFO("Using %s%s%s%s%s%s%s", kCipherSuites[curr_keypair_->cipher_suite], 
            curr_keypair_->enabled_features[WG_FEATURE_ID_SHORT_HEADER] ? ", short_header" : "",
            curr_keypair_->enabled_features[WG_FEATURE_ID_SHORT_MAC] ? ", mac64" : "",
            curr_keypair_->enabled_features[WG_FEATURE_ID_IPZIP] ? ", ipzip" : "",
            curr_keypair_->enabled_features[WG_FEATURE_ID_SKIP_KEYID_IN] ? ", skip_keyid_in" : "",
            curr_keypair_->enabled_features[WG_FEATURE_ID_SKIP_KEYID_OUT] ? ", skip_keyid_out" : "",
            curr_keypair_->enabled_features[WG_FEATURE_HYBRID_TCP] ? ", hybrid_tcp" : "");
    }
  }
  last_complete_handskake_timestamp_ = now;
//  RINFO("Connection established.");
}

// Check if any of the timeouts have expired
uint32 WgPeer::CheckTimeouts_Locked(uint64 now) {
  assert(dev_->IsMainThread() && IsPeerLocked());

  uint32 t, rv = 0;

  if (now >= time_of_next_key_event_)
    CheckAndUpdateTimeOfNextKeyEvent(now);

  if ((t = timers_) == 0)
    return 0;
  uint32 now32 = (uint32)now;
  // Got any new timers?
  if (t & (((1 << TIMERS_COUNT) - 1) << TIMERS_COUNT)) {
    if (t & (1 << (TIMERS_COUNT+0))) timer_value_[0] = now32;
    if (t & (1 << (TIMERS_COUNT+1))) timer_value_[1] = now32;
    if (t & (1 << (TIMERS_COUNT+2))) timer_value_[2] = now32;
    if (t & (1 << (TIMERS_COUNT+3))) timer_value_[3] = now32;
    if (t & (1 << (TIMERS_COUNT+4))) timer_value_[4] = now32;
    if (t & (1 << (TIMERS_COUNT+5))) timer_value_[5] = now32;
    t |= (t >> TIMERS_COUNT);
    t &= (1 << TIMERS_COUNT) - 1;
  }
  // Got any expired timers?
  if (t & ((1 << TIMERS_COUNT) - 1)) {
    if ((t & (1 << TIMER_RETRANSMIT_HANDSHAKE)) && (now32 - timer_value_[TIMER_RETRANSMIT_HANDSHAKE]) >= REKEY_TIMEOUT_MS) {
      t ^= (1 << TIMER_RETRANSMIT_HANDSHAKE);
      if (handshake_attempts_ > MAX_HANDSHAKE_ATTEMPTS || endpoint_.sin.sin_family == 0) {
        t &= ~(1 << TIMER_SEND_KEEPALIVE);
        ClearPacketQueue_Locked();
      } else {
        handshake_attempts_++;
        rv |= ACTION_SEND_HANDSHAKE;
      }
    }
    if ((t & (1 << TIMER_SEND_KEEPALIVE)) && (now32 - timer_value_[TIMER_SEND_KEEPALIVE]) >= keepalive_timeout_ms_) {
      // When header obfuscation is enabled, vary this between 7,8,9,10,11,12
      if (WITH_HEADER_OBFUSCATION && dev_->packet_obfuscator().enabled())
        keepalive_timeout_ms_ = KEEPALIVE_TIMEOUT_MS + ((int)(dev_->GetRandomNumber() % 6) - 3) * 1000;

      t &= ~(1 << TIMER_SEND_KEEPALIVE);
      rv |= ACTION_SEND_KEEPALIVE;
      if (pending_keepalive_) {
        pending_keepalive_ = false;
        timer_value_[TIMER_SEND_KEEPALIVE] = now32;
        t |= (1 << TIMER_SEND_KEEPALIVE);
      }
    }
    if ((t & (1 << TIMER_PERSISTENT_KEEPALIVE)) && (now32 - timer_value_[TIMER_PERSISTENT_KEEPALIVE]) >= (uint32)persistent_keepalive_ms_) {
      t &= ~(1 << TIMER_PERSISTENT_KEEPALIVE);
      if (persistent_keepalive_ms_) {
        t &= ~(1 << TIMER_SEND_KEEPALIVE);
        rv |= ACTION_SEND_KEEPALIVE;
      }
    }
    if ((t & (1 << TIMER_NEW_HANDSHAKE)) && (now32 - timer_value_[TIMER_NEW_HANDSHAKE]) >= KEEPALIVE_TIMEOUT_MS + REKEY_TIMEOUT_MS) {
      t &= ~(1 << TIMER_NEW_HANDSHAKE);
      if (endpoint_.sin.sin_family != 0) {
        handshake_attempts_ = 0;
        rv |= ACTION_SEND_HANDSHAKE;
      }
    }
    if ((t & (1 << TIMER_ZERO_KEYS)) && (now32 - timer_value_[TIMER_ZERO_KEYS]) >= REJECT_AFTER_TIME_MS * 3) {
      RINFO("Expiring all keys for peer");
      t &= ~(1 << TIMER_ZERO_KEYS);
      ClearKeys_Locked();
      ClearHandshake_Locked();
    }

    if ((t & (1 << TIMER_HYBRID_TCP)) && (now32 - timer_value_[TIMER_HYBRID_TCP]) >= HYBRID_TCP_TIMEOUT_MS) {
      t &= ~(1 << TIMER_HYBRID_TCP);
      // Forget about the data endpoint and switch to using the regular endpoint after 15 seconds.
      if (allow_endpoint_change_) {
        data_endpoint_protocol_ = endpoint_protocol_;
        data_endpoint_ = endpoint_;
      }
    }

  }
  timers_ = t;
  return rv;
}

// Check all key stuff here to avoid calling possibly expensive timestamp routines in the packet handler
void WgPeer::CheckAndUpdateTimeOfNextKeyEvent(uint64 now) {
  assert(dev_->IsMainThread() && IsPeerLocked());
  uint64 next_time = UINT64_MAX;
  uint32 rv = 0;

  if (curr_keypair_ != NULL) {
    if (now >= curr_keypair_->key_timestamp + REJECT_AFTER_TIME_MS) {
      DeleteKeypair(&curr_keypair_);
    } else if (curr_keypair_->is_initiator) {
      // if a peer is the initiator of a current secure session, WireGuard will send a handshake initiation
      // message to begin a new secure session if, after transmitting a transport data message, the current secure session
      // is REKEY_AFTER_TIME_MS old, or if after receiving a transport data message, the current secure session is
      // (REKEY_AFTER_TIME_MS - KEEPALIVE_TIMEOUT_MS - REKEY_TIMEOUT_MS) seconds old and it has not yet acted upon it.
      if (now >= curr_keypair_->key_timestamp + (REJECT_AFTER_TIME_MS - KEEPALIVE_TIMEOUT_MS - REKEY_TIMEOUT_MS)) {
        next_time = curr_keypair_->key_timestamp + REJECT_AFTER_TIME_MS;
        if (curr_keypair_->recv_key_state == WgKeypair::KEY_VALID)
          curr_keypair_->recv_key_state = WgKeypair::KEY_WANT_REFRESH;
      } else if (now >= curr_keypair_->key_timestamp + REKEY_AFTER_TIME_MS) {
        next_time = curr_keypair_->key_timestamp + (REJECT_AFTER_TIME_MS - KEEPALIVE_TIMEOUT_MS - REKEY_TIMEOUT_MS);
        if (curr_keypair_->send_key_state == WgKeypair::KEY_VALID)
          curr_keypair_->send_key_state = WgKeypair::KEY_WANT_REFRESH;
      } else  {
        next_time = curr_keypair_->key_timestamp + REKEY_AFTER_TIME_MS;
      }
    } else {
      next_time = curr_keypair_->key_timestamp + REJECT_AFTER_TIME_MS;
    }
  }
  if (prev_keypair_ != NULL) {
    if (now >= prev_keypair_->key_timestamp + REJECT_AFTER_TIME_MS)
      DeleteKeypair(&prev_keypair_);
    else
      next_time = std::min<uint64>(next_time, prev_keypair_->key_timestamp + REJECT_AFTER_TIME_MS);
  }
  if (next_keypair_ != NULL) {
    if (now >= next_keypair_->key_timestamp + REJECT_AFTER_TIME_MS)
      DeleteKeypair(&next_keypair_);
    else
      next_time = std::min<uint64>(next_time, next_keypair_->key_timestamp + REJECT_AFTER_TIME_MS);
  }
  time_of_next_key_event_ = next_time;
}

bool WgPeer::IsTransientDataEndpointActive() {
  return WgIsTimerActive(TIMER_HYBRID_TCP) != 0;
}

void WgPeer::SetEndpoint(int endpoint_proto, const IpAddr &sin) {
  endpoint_protocol_ = endpoint_proto;
  data_endpoint_protocol_ = endpoint_proto;
  endpoint_ = sin;
  data_endpoint_ = sin;
}

bool WgPeer::SetPersistentKeepalive(int persistent_keepalive_secs) {
  if (persistent_keepalive_secs < 0 || persistent_keepalive_secs > 65535)
    return false;
  persistent_keepalive_ms_ = persistent_keepalive_secs * 1000;
  return true;
}

bool WgCidrAddrEquals(const WgCidrAddr &a, const WgCidrAddr &b) {
  return (a.size == b.size && a.cidr == b.cidr && memcmp(a.addr, b.addr, a.size >> 3) == 0);
}

bool WgPeer::AddIp(const WgCidrAddr &cidr_addr) {
  WgPeer *old_peer;
  assert(dev_->IsMainThread());

  if (cidr_addr.size == 32) {
    if (cidr_addr.cidr > 32)
      return false;
    WG_ACQUIRE_RWLOCK_EXCLUSIVE(dev_->ip_to_peer_map_lock_);
    old_peer = (WgPeer*)dev_->ip_to_peer_map_.InsertV4(ReadBE32(cidr_addr.addr), cidr_addr.cidr, this);
    WG_RELEASE_RWLOCK_EXCLUSIVE(dev_->ip_to_peer_map_lock_);
  } else if (cidr_addr.size == 128) {
    if (cidr_addr.cidr > 128)
      return false;
    WG_ACQUIRE_RWLOCK_EXCLUSIVE(dev_->ip_to_peer_map_lock_);
    old_peer = (WgPeer*)dev_->ip_to_peer_map_.InsertV6(cidr_addr.addr, cidr_addr.cidr, this);
    WG_RELEASE_RWLOCK_EXCLUSIVE(dev_->ip_to_peer_map_lock_);
  } else {
    return false;
  }
  if (old_peer) {
    for (auto it = old_peer->allowed_ips_.begin(); it != old_peer->allowed_ips_.end(); ++it) {
      if (WgCidrAddrEquals(*it, cidr_addr)) {
        old_peer->allowed_ips_.erase(it);
        break;
      }
    }
  }
  allowed_ips_.push_back(cidr_addr);
  return true;
}

void WgPeer::RemoveAllIps() {
  assert(dev_->IsMainThread());
  WG_ACQUIRE_RWLOCK_EXCLUSIVE(dev_->ip_to_peer_map_lock_);
  for (auto it = allowed_ips_.begin(); it != allowed_ips_.end(); ++it) {
    if (it->size == 32) {
      dev_->ip_to_peer_map_.RemoveV4(ReadBE32(it->addr), it->cidr);
    } else if (it->size == 128) {
      dev_->ip_to_peer_map_.RemoveV6(it->addr, it->cidr);
    }
  }
  WG_RELEASE_RWLOCK_EXCLUSIVE(dev_->ip_to_peer_map_lock_);
  allowed_ips_.clear();
}

void WgPeer::SetAllowMulticast(bool allow) {
  allow_multicast_through_peer_ = allow;
}

void WgPeer::SetFeature(int feature, uint8 value) {
  features_[feature] = value;
}

bool WgPeer::AddCipher(int cipher) {
  if (num_ciphers_ == MAX_CIPHERS)
    return false;

  if (cipher == EXT_CIPHER_SUITE_AES128_GCM || cipher == EXT_CIPHER_SUITE_AES256_GCM) {
#if defined(ARCH_CPU_X86_FAMILY) && WITH_AESGCM
    if (!X86_PCAP_AES)
      return true;
#else
    return true;
#endif  // defined(ARCH_CPU_X86_FAMILY) && WITH_AESGCM
  }
  ciphers_[num_ciphers_++] = cipher;
  return true;
}

void WgPeer::ScheduleNewHandshake() {
  // Note, it's possible that the peer has already been marked for delete
  if (main_thread_scheduled_.fetch_or(WgPeer::kMainThreadScheduled_ScheduleHandshake) == 0) {
    main_thread_scheduled_next_ = NULL;
    WG_ACQUIRE_LOCK(dev_->main_thread_scheduled_lock_);
    *dev_->main_thread_scheduled_last_ = this;
    dev_->main_thread_scheduled_last_ = &main_thread_scheduled_next_;
    WG_RELEASE_LOCK(dev_->main_thread_scheduled_lock_);
    // todo: in multithreaded impl need to trigger |RunAllMainThreadScheduled| to get called
  }
}

WgRateLimit::WgRateLimit() { 
  key1_[0] = key1_[1] = 1;
  key2_[0] = key2_[1] = 1;
  bin1_ = bins_[0];
  bin2_ = bins_[1];
  rand_ = 0;
  rand_xor_ = 0;
  packets_per_sec_ = PACKETS_PER_SEC;
  used_rate_limit_ = 0;
  memset(bins_, 0, sizeof(bins_));
}

void WgRateLimit::Periodic(uint32 s[5]) {
  unsigned int per_sec = PACKETS_PER_SEC;
  if (used_rate_limit_ >= TOTAL_PACKETS_PER_SEC) {
    per_sec = PACKETS_PER_SEC * TOTAL_PACKETS_PER_SEC / used_rate_limit_;
    if (per_sec < 1)
      per_sec = 1;
  }
  if ((unsigned)per_sec > packets_per_sec_)
    per_sec = (per_sec + packets_per_sec_ + 1) >> 1;

  packets_per_sec_ = per_sec;  
  used_rate_limit_ = 0;
  rand_xor_ = s[4];
  key2_[0] = key1_[0];
  key2_[1] = key1_[1];
  memcpy(key1_, s, sizeof(key1_));
  std::swap(bin1_, bin2_);
  memset(bin1_, 0, BINSIZE);
}

static inline size_t hashit(uint64 ip, const uint64 *key) {
  uint64 x = ip * key[0] + rol64(ip, 32) * key[1];
  uint32 a = (uint32)(x + (x >> 32) * 0x85ebca6b);
  a -= a >> 16;
  a ^= a >> 4;
  return a;
}

WgRateLimit::RateLimitResult WgRateLimit::CheckRateLimit(uint64 ip) {
  uint8 *a = &bin1_[hashit(ip, key1_) & (BINSIZE - 1)];
  uint8 *b = &bin2_[hashit(ip, key2_) & (BINSIZE - 1)];
  unsigned int old = std::max<int>(*a, *b - packets_per_sec_), v = 0;
  if (old < PACKET_ACCUM / 2) {
    v = 1;
  } else if (old < PACKET_ACCUM) {
    v = old < ((uint64)rand_ * ((PACKET_ACCUM / 2) + 1) >> 32) + (PACKET_ACCUM / 2);
    rand_ = (rand_ * 0x1b873593 + 5) + rand_xor_;
  }
  RateLimitResult rr = {a, (uint8)(old + v), (uint8)v};
  return rr;
}

void WgKeypairEncryptPayload(uint8 *dst, const size_t src_len,
    const uint8 *ad, const size_t ad_len,
    const uint64 nonce, WgKeypair *keypair) {
  if (keypair->cipher_suite == EXT_CIPHER_SUITE_CHACHA20POLY1305) {
    chacha20poly1305_encrypt(dst, dst, src_len, ad, ad_len, nonce, keypair->send_key);
  } else if (keypair->cipher_suite >= EXT_CIPHER_SUITE_AES128_GCM && keypair->cipher_suite <= EXT_CIPHER_SUITE_AES256_GCM) {
#if WITH_AESGCM
    aesgcm_encrypt(dst, dst, src_len, ad, ad_len, nonce, &keypair->aes_gcm128_context_[0]);
#endif  // WITH_AESGCM
  } else {
    poly1305_get_mac(dst, src_len, ad, ad_len, nonce, keypair->send_key, dst + src_len);
  }

  // Convert MAC to 8 bytes if that's all we need.
  if (keypair->auth_tag_length != WG_MAC_LEN) {
    uint8 *mac = dst + src_len;
    uint64 rv = siphash_2u64(ReadLE64(mac), ReadLE64(mac + 8), (siphash_key_t*)keypair->compress_mac_keys[0]);
    WriteLE64(mac, rv);
  }
}

bool WgKeypairDecryptPayload(uint8 *dst, size_t src_len,
    const uint8 *ad, size_t ad_len,
    const uint64 nonce, WgKeypair *keypair) {

  __aligned(16) uint8 mac[16];

  if (src_len < keypair->auth_tag_length)
    return false;

  src_len -= keypair->auth_tag_length;

  if (keypair->cipher_suite == EXT_CIPHER_SUITE_CHACHA20POLY1305) {
    chacha20poly1305_decrypt_get_mac(dst, dst, src_len, ad, ad_len, nonce, keypair->recv_key, mac);
  } else if (keypair->cipher_suite >= EXT_CIPHER_SUITE_AES128_GCM && keypair->cipher_suite <= EXT_CIPHER_SUITE_AES256_GCM) {
#if WITH_AESGCM
    aesgcm_decrypt_get_mac(dst, dst, src_len, ad, ad_len, nonce, &keypair->aes_gcm128_context_[1], mac);
#else   // WITH_AESGCM
    return false;
#endif  // WITH_AESGCM
  } else {
    poly1305_get_mac(dst, src_len, ad, ad_len, nonce, keypair->recv_key, mac);
  }

  if (keypair->auth_tag_length == WG_MAC_LEN) {
    return memcmp_crypto(mac, dst + src_len, WG_MAC_LEN) == 0;
  } else {
    uint64 rv = siphash_2u64(ReadLE64(mac), ReadLE64(mac + 8), (siphash_key_t*)keypair->compress_mac_keys[1]);
    WriteLE64(mac, rv);
    return memcmp_crypto(mac, dst + src_len, keypair->auth_tag_length) == 0;
  }
}

// A random siphash key that can be used for hashing so it gets harder to induce hash collisions.
struct RandomSiphashKey {
  RandomSiphashKey() { OsGetRandomBytes((uint8*)&key, sizeof(key)); }
  siphash_key_t key;
};
static RandomSiphashKey random_siphash_key;

size_t WgAddrEntry::IpPortHasher::operator()(const WgAddrEntry::IpPort &a) const {
  uint32 xx = Read32(a.bytes + 16);
  return (size_t)siphash13_2u64(Read64(a.bytes) + xx, Read64(a.bytes + 8) + xx, &random_siphash_key.key);
}

size_t WgPublicKeyHasher::operator()(const WgPublicKey&a) const {
  return (size_t)siphash13_4u64(a.u64[0], a.u64[1], a.u64[2], a.u64[3], &random_siphash_key.key);
}

// This scrambles the initial 16 bytes of the packet with the
// last 8 bytes of the packet as a seed.
void WgPacketObfuscator::ScrambleUnscramble(uint8 *data, size_t data_size) {
  assert(data_size >= 16);

  uint64 last_uint64 = ReadLE64(data + data_size - 8);
  uint64 a = siphash_u64_u32(last_uint64, (uint32)data_size, (siphash_key_t*)&key_[0]);
  uint64 b = siphash_u64_u32(last_uint64, (uint32)data_size, (siphash_key_t*)&key_[2]);
  ((uint64*)data)[0] ^= ToLE64(a);
  b = ToLE64(b);
  if (data_size >= 24) {
    ((uint64*)data)[1] ^= b;
  } else {
    uint64 d[1] = { b };
    for (size_t i = 0; i < data_size - 16; i++)
      data[i + 8] ^= ((uint8*)d)[i];
  }
}

size_t WgPacketObfuscator::InsertRandomBytesIntoPacket(uint8 *data, size_t data_size) {
  assert(data_size >= 24);
  // The bytes at offset 16 are used as a seed to the prng
  uint64 master_key = siphash_u64_u32(Read64(data + 16), (uint32)data_size, &random_siphash_key.key);
  uint32 random_bytes = master_key & 0xFF;
  data[3] = (uint8)random_bytes;
  for (uint32 i = 0; i < random_bytes; i += 8)
    *(uint64*)(data + data_size + i) = siphash_u64_u32(master_key + i, i, &random_siphash_key.key);
  data_size += random_bytes;
  return data_size;
}

void WgPacketObfuscator::ObfuscatePacket(Packet *packet) {
  uint8 *data = packet->data;
  size_t data_size = packet->size;

  // Too short packets can't be obfuscated
  if (data_size < 8)
    return;
  
  // If the packet is type 1, 2 or 3, or a keepalive packet of type 4, add random bytes at
  // the end. This is to make it harder to detect the protocol. Store the # of added bytes
  // in the 3:rd byte of the packet.
  uint32 packet_type = ReadLE32(data);
  if ((packet_type == 4 && data_size <= 32) || packet_type < 4) {
    // The 39:th (for handshake init) and 43:rd byte (for handshake response)
    // have zero MSB because of curve25519 pubkey, so xor it with random.
    if (packet_type < 4) {
      assert(data_size >= 48);
      data[35 + packet_type * 4] ^= data[15];
    }
    packet->size = (uint)(data_size = InsertRandomBytesIntoPacket(data, data_size));
  }

  // Scramble the header bytes of the packet
  ScrambleUnscramble(data, data_size);
}

void WgPacketObfuscator::DeobfuscatePacket(Packet *packet) {
  uint8 *data = packet->data;
  size_t data_size = packet->size;

  // Too short packets can't be obfuscated / deobfuscated
  if (data_size < 8)
    return;

  // Unscramble the header bytes of the packet
  ScrambleUnscramble(data, data_size);

  // Check whether the packet type field says that we have 
  // extra bytes appended at the end.
  if (data[0] <= 4) {
    if (data[3] > data_size)
      return; // invalid
    packet->size = (uint32)(data_size -= data[3]);
    data[3] = 0;
    // The 39:th (for handshake init) and 43:rd byte (for handshake response)
    // have zero MSB because of curve25519 pubkey, so xor it with random.
    if (data[0] < 4 && data_size >= 48)
      data[35 + data[0] * 4] ^= data[15];
  }
}


//>> > hashlib.sha256('TunSafe Header Obfuscation Key').hexdigest()
//'2444423e33eb5bb875961224c6441f54c5dea95a3a4e1139509ffa6992bdb278'
static const uint8 kHeaderObfuscationKey[32] = { 36, 68, 66, 62, 51, 235, 91, 184, 117, 150, 18, 36, 198, 68, 31, 84, 197, 222, 169, 90, 58, 78, 17, 57, 80, 159, 250, 105, 146, 189, 178, 120 };

void WgPacketObfuscator::SetKey(const uint8 *key, size_t len) {
  enabled_ = (key != NULL);
  if (key)
    blake2s((uint8*)&key_, sizeof(key_), key, len, kHeaderObfuscationKey, sizeof(kHeaderObfuscationKey));
}
