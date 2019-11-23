#include "stdafx.h"
#include "network_common.h"
#include "netapi.h"
#include "tunsafe_endian.h"
#include <assert.h>
#include <algorithm>
#include "util.h"
#include "crypto/chacha20poly1305.h"
#include "crypto/blake2s/blake2s.h"
#include "wireguard_proto.h"

enum {
  CRYPTO_HEADER_SIZE = 64,
};

enum {
  READ_CRYPTO_HEADER = 0,
  READ_PACKET_HEADER = 1,
  READ_PACKET_DATA = 2,
};

TcpPacketQueue::~TcpPacketQueue() {
  FreePacketList(rqueue_);
}

Packet *TcpPacketQueue::Read(uint num) {
  // Move data around to ensure that exactly the first |num| bytes are stored
  // in the first packet, and the rest of the data in subsequent packets.
  Packet *p = rqueue_;

  assert(num <= kPacketCapacity);
  if (p->size < num) {
    // There's not enough data in the current packet, copy data from the next packet
    // into this packet.
    if ((uint)(&p->data_buf[kPacketCapacity] - p->data) < num) {
      // Move data up front to make space.
      memmove(p->data_buf, p->data, p->size);
      p->data = p->data_buf;
    }
    // Copy data from future packets into p, and delete them should they become empty.
    do {
      Packet *n = Packet_NEXT(p);
      uint bytes_to_copy = std::min(n->size, num - p->size);
      uint nsize = (n->size -= bytes_to_copy);
      memcpy(p->data + postinc(p->size, bytes_to_copy), postinc(n->data, bytes_to_copy), bytes_to_copy);
      if (nsize == 0) {
        p->queue_next = n->queue_next;
        pool_->FreePacketToPool(n);
      }
    } while (num - p->size);
  } else if (p->size > num) {
    // The packet has too much data. Split the packet into two packets.
    Packet *n = pool_->AllocPacketFromPool();
    if (!n)
      return NULL; // unable to allocate a packet....?
    if (num * 2 <= p->size) {
      // There's a lot of trailing data: PP NNNNNN. Move PP.
      n->size = num;
      p->size -= num;
      rqueue_bytes_ -= num;
      memcpy(n->data, postinc(p->data, num), num);
      return n;
    } else {
      uint overflow = p->size - num;
      // There's a lot of leading data: PPPPPP NN. Move NN
      n->size = overflow;
      p->size = num;
      rqueue_ = n;
      if (!(n->queue_next = p->queue_next))
        rqueue_end_ = &Packet_NEXT(n);
      rqueue_bytes_ -= num;
      memcpy(n->data, p->data + num, overflow);
      return p;
    }
  }
  if ((rqueue_ = Packet_NEXT(p)) == NULL)
    rqueue_end_ = &rqueue_;
  rqueue_bytes_ -= num;
  return p;
}

Packet *TcpPacketQueue::ReadUpTo(uint num) {
  assert(rqueue_bytes_ != 0);
  Packet *p = rqueue_;
  if (num < p->size)
    return Read(num);
  rqueue_bytes_ -= p->size;
  if ((rqueue_ = Packet_NEXT(p)) == NULL)
    rqueue_end_ = &rqueue_;
  return p;
}

void TcpPacketQueue::Add(Packet *p) {
  assert(p->size != 0);
  rqueue_bytes_ += p->size;
  p->queue_next = NULL;
  *rqueue_end_ = p;
  rqueue_end_ = &Packet_NEXT(p);
}

void TcpPacketQueue::Read(uint8 *dst, uint size) {
  assert(size <= rqueue_bytes_);
  rqueue_bytes_ -= size;
  while (size) {
    Packet *packet = rqueue_;
    uint n = std::min(packet->size, size);
    uint8 *src = packet->data;
    for (uint i = 0; i != n; i++)
      *dst++ = *src++;
    packet->data = src;
    size -= n;
    if ((packet->size -= n) == 0) {
      if ((rqueue_ = Packet_NEXT(packet)) == NULL)
        rqueue_end_ = &rqueue_;
      pool_->FreePacketToPool(packet);
    }
  }
}

uint TcpPacketQueue::PeekUint16() {
  return (rqueue_->size >= 2) ? ReadBE16(rqueue_->data) :
           (rqueue_->data[0] << 8) + Packet_NEXT(rqueue_)->data[0];
}

TcpPacketHandler::TcpPacketHandler(SimplePacketPool *packet_pool, WgPacketObfuscator *obfuscator, bool is_incoming)
   : queue_(packet_pool),
     tls_queue_(packet_pool),
     write_state_(is_incoming),
     obfuscation_mode_(kObfuscationMode_None) {

  if (obfuscator->enabled() && obfuscator->obfuscate_tcp() != TcpPacketHandler::kObfuscationMode_None) {
    memcpy(encryptor_.buf, obfuscator->key(), CHACHA20POLY1305_KEYLEN);
    memcpy(decryptor_.buf, obfuscator->key(), CHACHA20POLY1305_KEYLEN);
    obfuscation_mode_ = obfuscator->obfuscate_tcp() != TcpPacketHandler::kObfuscationMode_Unspecified ? obfuscator->obfuscate_tcp() :
        (is_incoming ? TcpPacketHandler::kObfuscationMode_Autodetect : TcpPacketHandler::kObfuscationMode_Encrypted);
    read_state_ = (obfuscation_mode_ == kObfuscationMode_Encrypted) ? READ_CRYPTO_HEADER : READ_PACKET_HEADER;
  } else if (!obfuscator->enabled() && obfuscator->obfuscate_tcp() > TcpPacketHandler::kObfuscationMode_None) {
    RERROR("No ObfuscateKey specified. Disabling TCP obfuscation.");
  }
  tls_read_state_ = 0;
  error_flag_ = false;
  decryptor_initialized_ = false;
  predicted_key_in_ = predicted_key_out_ = 0;
  predicted_serial_in_ = predicted_serial_out_ = 0;
}

TcpPacketHandler::~TcpPacketHandler() {
}

enum {
  kTcpPacketType_Normal = 0,
  kTcpPacketType_Reserved = 1,
  kTcpPacketType_Data = 2,
  kTcpPacketType_Control = 3,
  kTcpPacketControlType_SetKeyAndCounter = 0,
};

static void SetChachaStreamingKey(chacha20_streaming *chacha, const uint8 *key, size_t key_len) {
  blake2s(chacha->buf, CHACHA20POLY1305_KEYLEN, key, key_len, chacha->buf, CHACHA20POLY1305_KEYLEN);
  chacha20_streaming_init(chacha, chacha->buf);
}

size_t TcpPacketHandler::CreateTls13ClientHello(uint8 *dst) {
  uint8 *dst_org = dst;
  // handshake, tls 1.0
  *dst++ = 0x16;
  *dst++ = 0x03;
  *dst++ = 0x01;
  uint8 *handshake_length = postinc(dst, 2);
  // handshake client hello
  *dst++ = 0x01;
  *dst++ = 0x00;
  uint8 *handshake_inner_length = postinc(dst, 2);
  // version = tls 1.2
  *dst++ = 0x03;
  *dst++ = 0x03;
  // 32 byte random
  OsGetRandomBytes(postinc(dst, 32), 32);
  *dst++ = 0x20;   // Session length = 32
  // 32 byte session id
  OsGetRandomBytes(postinc(dst, 32), 32);

  bool firefox = (obfuscation_mode_ == kObfuscationMode_TlsFirefox);

  if (firefox) {
    static const uint8 tls_header1[] = {
      // 18 cipher suites
      0x00, 0x24,
      0x13, 0x01, 0x13, 0x03, 0x13, 0x02, 0xc0, 0x2b, 0xc0, 0x2f, 0xcc, 0xa9, 0xcc, 0xa8, 0xc0, 0x2c, 0xc0, 0x30,
      0xc0, 0x0a, 0xc0, 0x09, 0xc0, 0x13, 0xc0, 0x14, 0x00, 0x33, 0x00, 0x39, 0x00, 0x2f, 0x00, 0x35, 0x00, 0x0a,
      // compression method = null
      0x01, 0x00,
    };
    memcpy(postinc(dst, sizeof(tls_header1)), tls_header1, sizeof(tls_header1));
  } else {
    static const uint8 tls_header1_chrome[] = {
      // 17 cipher suites
      0x00, 0x22,
      0xda, 0xda, 0x13, 0x01, 0x13, 0x02, 0x13, 0x03, 0xc0, 0x2b, 0xc0, 0x2f, 0xc0, 0x2c, 0xc0, 0x30, 0xcc, 0xa9,
      0xcc, 0xa8, 0xc0, 0x13, 0xc0, 0x14, 0x00, 0x9c, 0x00, 0x9d, 0x00, 0x2f, 0x00, 0x35, 0x00, 0x0a,
      // compression method = null
      0x01, 0x00,
    };
    memcpy(postinc(dst, sizeof(tls_header1_chrome)), tls_header1_chrome, sizeof(tls_header1_chrome));

  }
  uint8 *extensions_length = postinc(dst, 2);

  if (!firefox) {
    static const uint8 tls_header_grease[] = { 0xaa, 0xaa, 0x00, 0x00 };
    memcpy(postinc(dst, sizeof(tls_header_grease)), tls_header_grease, sizeof(tls_header_grease));
  }

  static const uint8 tls_header2[] = {
    // extension server name
    0x00, 0x00, 0x00, 0x16, 0x00, 0x14, 0x00, 0x00, 0x11, 0x65, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x64, 0x2e, 0x74, 0x6c, 0x73, 0x31, 0x33, 0x2e, 0x63, 0x6f, 0x6d,
    // extension master secret
    0x00, 0x17, 0x00, 0x00,
    // extension renegotiation info
    0xff, 0x01, 0x00, 0x01, 0x00,
  };
  memcpy(postinc(dst, sizeof(tls_header2)), tls_header2, sizeof(tls_header2));

  if (firefox) {
    static const uint8 tls_header_groups_ff[] = {
      // extension supported groups
      0x00, 0x0a, 0x00, 0x0e, 0x00, 0x0c,
      0x00, 0x1d, 0x00, 0x17, 0x00, 0x18, 0x00, 0x19, 0x01, 0x00, 0x01, 0x01,
      // extension ec_point_formats
      0x00, 0x0b, 0x00, 0x02, 0x01, 0x00,
      // extension application_layer_protocol_negotiation
      0x00, 0x10, 0x00, 0x0e, 0x00, 0x0c, 0x02, 0x68, 0x32, 0x08, 0x68, 0x74, 0x74, 0x70, 0x2f, 0x31, 0x2e, 0x31,
      // extension status request
      0x00, 0x05, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00,
      // extension key share
      0x00, 0x33, 0x00, 0x6b, 0x00, 0x69,
      // key share x25519
      0x00, 0x1d, 0x00, 0x20,
    };
    memcpy(postinc(dst, sizeof(tls_header_groups_ff)), tls_header_groups_ff, sizeof(tls_header_groups_ff));
    // Firefox has a secp251p1 key while chrome does not
    OsGetRandomBytes(postinc(dst, 32), 32);
    dst[-1] &= 0x7f; // clear top bit of x25519 key
    static const uint8 tls_header3[] = {
      // key share secp256p1
      0x00, 0x17, 0x00, 0x41,
      0x04,
    };
    memcpy(postinc(dst, sizeof(tls_header3)), tls_header3, sizeof(tls_header3));
    // todo: validate the secp256p1 key
    OsGetRandomBytes(postinc(dst, 64), 64);

    static const uint8 tls_header4[] = {
      // extension early data (seems to be sent only in resume)
      0x00, 0x2a, 0x00, 0x00,
      // extension supported versions
      0x00, 0x2b, 0x00, 0x09, 0x08, 0x03, 0x04, 0x03, 0x03, 0x03, 0x02, 0x03, 0x01,
      // extension signature_algorithms
      0x00, 0x0d, 0x00, 0x18, 0x00, 0x16, 0x04, 0x03, 0x05, 0x03, 0x06, 0x03, 0x08, 0x04, 0x08, 0x05, 0x08, 0x06, 0x04, 0x01, 0x05, 0x01, 0x06, 0x01, 0x02, 0x03, 0x02, 0x01,
      // extension psk_key_exchange_modes
      0x00, 0x2d, 0x00, 0x02, 0x01, 0x01,
      // extension unknown type 28
      0x00, 0x1c, 0x00, 0x02, 0x40, 0x01,
      // extension pre shared key length=235
      0x00, 0x29, 0x00, 0xeb,
      // identities length=198, psk identity length = 192
      0x00, 0xc6, 0x00, 0xc0,
    };
    memcpy(postinc(dst, sizeof(tls_header4)), tls_header4, sizeof(tls_header4));

  } else {
    static const uint8 tls_header_groups_chrome[] = {
      // extension supported groups
      0x00, 0x0a, 0x00, 0x0a, 0x00, 0x08, 0x2a, 0x2a, 0x00, 0x1d, 0x00, 0x17, 0x00, 0x18,
      // extension ec_point_formats
      0x00, 0x0b, 0x00, 0x02, 0x01, 0x00,
      // extension sessionticket tls
      0x00, 0x23, 0x00, 0x00,
      // extension application_layer_protocol_negotiation
      0x00, 0x10, 0x00, 0x0e, 0x00, 0x0c, 0x02, 0x68, 0x32, 0x08, 0x68, 0x74, 0x74, 0x70, 0x2f, 0x31, 0x2e, 0x31,
      // extension status request
      0x00, 0x05, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00,
      // extension signature_algorithms
      0x00, 0x0d, 0x00, 0x14, 0x00, 0x12, 0x04, 0x03, 0x08, 0x04, 0x04, 0x01, 0x05, 0x03, 0x08, 0x05, 0x05, 0x01, 0x08, 0x06, 0x06, 0x01, 0x02, 0x01,
      // extension signed_certificate_timestamp
      0x00, 0x12, 0x00, 0x00,
      // extension key_share
      0x00, 0x33, 0x00, 0x2b, 0x00, 0x29, 
      0x2a, 0x2a, 0x00, 0x01, 0x00,
      0x00, 0x1d, 0x00, 0x20,
    };
    memcpy(postinc(dst, sizeof(tls_header_groups_chrome)), tls_header_groups_chrome, sizeof(tls_header_groups_chrome));
    OsGetRandomBytes(postinc(dst, 32), 32);
    dst[-1] &= 0x7f; // clear top bit of x25519 key

    static const uint8 tls_header4_chrome[] = {
      // extension psk_key_exchange_modes
      0x00, 0x2d, 0x00, 0x02, 0x01, 0x01,
      // extension supported versions
      0x00, 0x2b, 0x00, 0x0b, 0x0a, 0x1a, 0x1a, 0x03, 0x04, 0x03, 0x03, 0x03, 0x02, 0x03, 0x01,
      // extension unknown type 27
      0x00, 0x1b, 0x00, 0x03, 0x02, 0x00, 0x02,
      // extension reserved (grease)
      0xea, 0xea, 0x00, 0x01, 0x00,
      // extension pre shared key length=235
      0x00, 0x29, 0x00, 0xeb,
      // identities length=198, psk identity length = 192
      0x00, 0xc6, 0x00, 0xc0,
    };
    memcpy(postinc(dst, sizeof(tls_header4_chrome)), tls_header4_chrome, sizeof(tls_header4_chrome));
  }

  OsGetRandomBytes(postinc(dst, 192 + 4), 192 + 4);
  static const uint8 tls_header5[] = {
    // psk binders length
    0x00, 0x21,
  };
  memcpy(postinc(dst, sizeof(tls_header5)), tls_header5, sizeof(tls_header5));
  OsGetRandomBytes(postinc(dst, 33), 33);

  // Fixup lengths
  WriteBE16(handshake_length, (uint)(dst - dst_org - 5));
  WriteBE16(handshake_inner_length, (uint)(dst - dst_org - 9));
  WriteBE16(extensions_length, (uint)(dst - extensions_length - 2));
  
  // Setup the key generator for outgoing packets. It will be the blake2s hash of
  // the full message excluding the tls header.
  SetChachaStreamingKey(&encryptor_, dst_org + 5, dst - dst_org - 5);

  static const uint8 tls_header6[] = {
    // change cipher spec
    0x14, 0x03, 0x03, 0x00, 0x01, 0x01
  };
  memcpy(postinc(dst, sizeof(tls_header6)), tls_header6, sizeof(tls_header6));

  return dst - dst_org;
}

size_t TcpPacketHandler::CreateTls13ServerHello(uint8 *dst) {
  if (!decryptor_initialized_)
    return ~(size_t)0;

  uint8 *dst_org = dst;
  // handshake, tls 1.0
  *dst++ = 0x16;
  *dst++ = 0x03;
  *dst++ = 0x03;
  uint8 *handshake_length = postinc(dst, 2);
  // handshake client hello
  *dst++ = 0x02;
  *dst++ = 0x00;
  uint8 *handshake_inner_length = postinc(dst, 2);
  // version = tls 1.2
  *dst++ = 0x03;
  *dst++ = 0x03;
  // 32 byte random
  OsGetRandomBytes(postinc(dst, 32), 32);
  *dst++ = 0x20;   // Session length = 32
  // 32 byte session id taken from client hello.
  memcpy(postinc(dst, 32), tls_session_id_, 32);
  // cipher suite
  *dst++ = 0x13;
  *dst++ = 0x01;
  // compression method
  *dst++ = 0x00;

  uint8 *extensions_length = postinc(dst, 2);
  static const uint8 tls_s_header0[] = {
    // extension pre_shared_key
    0x00, 0x29, 0x00, 0x02, 0x00, 0x00,
    // extension key share with x25519 key
    0x00, 0x33, 0x00, 0x24, 0x00, 0x1d, 0x00, 0x20,
  };
  memcpy(postinc(dst, sizeof(tls_s_header0)), tls_s_header0, sizeof(tls_s_header0));
  OsGetRandomBytes(postinc(dst, 32), 32);
  dst[-1] &= 0x7f; // clear top bit of x25519 key

  static const uint8 tls_s_header1[] = {
    // extension supported version tls1.3
    0x00, 0x2b, 0x00, 0x02, 0x03, 0x04,
  };
  memcpy(postinc(dst, sizeof(tls_s_header1)), tls_s_header1, sizeof(tls_s_header1));

  WriteBE16(handshake_length, (uint)(dst - dst_org - 5));
  WriteBE16(handshake_inner_length, (uint)(dst - dst_org - 9));
  WriteBE16(extensions_length, (uint)(dst - extensions_length - 2));

  // Setup the key generator for outgoing packets. It will be the blake2s hash of
  // the full message excluding the tls header.
  SetChachaStreamingKey(&encryptor_, dst_org + 5, dst - dst_org - 5);

  static const uint8 tls_header6[] = {
    // change cipher spec
    0x14, 0x03, 0x03, 0x00, 0x01, 0x01
  };
  memcpy(postinc(dst, sizeof(tls_header6)), tls_header6, sizeof(tls_header6));

  return dst - dst_org;
}

// Normal packet without obfuscation
void TcpPacketHandler::PrepareOutgoingPacketsNormal(Packet *p) {
  uint8 *data = p->data;
  uint data_size = p->size, packet_type = ReadLE32(data);
  p->prepared = true;
  if (packet_type == 4) {
    assert(data_size >= 16);
    uint32 key = Read32(data + 4);
    uint64 serial = ReadLE64(data + 8);
    if (((predicted_key_out_ ^ key) | (exch(predicted_serial_out_, serial) ^ (serial - 1))) == 0) {
      p->data = data + 14;
      p->size = data_size - 14;
      WriteBE16(p->data, 0x8000 + data_size - 16);
      return;
    }
    predicted_key_out_ = key;
  }
  p->size = data_size + 2;
  p->data = data - 2;
  WriteBE16(p->data, data_size);
}

// Obfuscated stream that looks totally random
void TcpPacketHandler::PrepareOutgoingPacketsObfuscate(Packet *p) {
  uint8 *data = p->data;
  uint data_size = p->size, packet_type = ReadLE32(data);
  p->prepared = true;
  // When obfuscation is enabled, inject random shit into packets.
  if ((packet_type == 4 && data_size <= 32) || packet_type < 4) {
    if (packet_type != 4) {
      assert(data_size >= 48);
      // The 39:th (for handshake init) and 43:rd byte (for handshake response)
      // have zero MSB because of curve25519 pubkey, so xor it with random.
      if (packet_type < 4)
        data[35 + packet_type * 4] ^= data[15];
    } else {
      predicted_key_out_ = Read32(data + 4);
      predicted_serial_out_ = ReadLE64(data + 8);
    }
    data_size = (uint)WgPacketObfuscator::InsertRandomBytesIntoPacket(data, data_size);
  } else if (packet_type == 4) {
    assert(data_size >= 16);
    uint32 key = Read32(data + 4);
    uint64 serial = ReadLE64(data + 8);
    if (((exch(predicted_key_out_, key) ^ key) | (exch(predicted_serial_out_, serial) ^ (serial - 1))) == 0) {
      p->data = data + 14;
      p->size = data_size - 14;
      WriteBE16(p->data, 0x8000 + data_size - 16);
      chacha20_streaming_crypt(&encryptor_, p->data, 2);
      return;
    }
  }
  p->data = data - 2;
  p->size = data_size + 2;
  WriteBE16(p->data, data_size);
  chacha20_streaming_crypt(&encryptor_, p->data, 18);
}

static void PrependTlsApplicationData(Packet *p, uint data_size) {
  p->size += 5;
  p->data -= 5;
  p->data[0] = 0x17;
  p->data[1] = 0x03;
  p->data[2] = 0x03;
  p->data[4] = (uint8)data_size;
  p->data[3] = (uint8)(data_size >> 8);
}

void TcpPacketHandler::PrepareOutgoingPacketsTLS13(Packet *p) {
  // Collect a number of packets, but add just a single TLS header
  uint total_size = 0;
  Packet *cur = p;
  do {
    PrepareOutgoingPacketsObfuscate(cur);
    total_size += cur->size;
  } while (total_size < 12000 && (cur = Packet_NEXT(cur)));
  PrependTlsApplicationData(p, total_size);
}

Packet *TcpPacketHandler::GetNextWireguardPacketObfuscate(TcpPacketQueue *queue) {
  if (read_state_ == READ_CRYPTO_HEADER) {
    // Wait for the 64 bytes of crypto header, they will
    // be used to seed the decryptor.
    if (queue->size() < CRYPTO_HEADER_SIZE)
      return NULL;
    Packet *packet = queue->Read(CRYPTO_HEADER_SIZE);
    if (!packet)
      return NULL;
    SetChachaStreamingKey(&decryptor_, packet->data, CRYPTO_HEADER_SIZE);
    queue->pool()->FreePacketToPool(packet);
    read_state_ = READ_PACKET_HEADER;
  } else if (read_state_ == READ_PACKET_DATA) {
    goto case_READ_PACKET_DATA;
  }

  while (queue->size() >= 2) {
    // Peek and decrypt the packet header
    queue->Read(packet_header_, 2);
    chacha20_streaming_crypt(&decryptor_, packet_header_, 2);
case_READ_PACKET_DATA:
    uint32 packet_header = ReadBE16(packet_header_);
    uint32 packet_size = packet_header & 0x7FFF;
    if (packet_size > kPacketCapacity) {
error:
      error_flag_ = true;
      return NULL;
    }
    if (packet_size > queue->size()) {
      read_state_ = READ_PACKET_DATA;
      return NULL;
    }
    read_state_ = READ_PACKET_HEADER;
    Packet *packet = queue->Read(packet_size);
    if (!packet)
      goto error;
    //      RINFO("Packet of type %d, size %d", packet_type, packet->size - 2);
    if (!(packet_header & 0x8000)) {
      unsigned int size = packet->size;
      // decrypt the initial 16 bytes of the packet
      if (size < 16)
        goto error;
      chacha20_streaming_crypt(&decryptor_, packet->data, 16);
      // Discard any extra junk bytes appended at the end.
      if (packet->data[0] <= 4) {
        if (packet->data[3] > size)
          goto error;
        packet->size = (size -= packet->data[3]);
        packet->data[3] = 0;
        // The 39:th (for handshake init) and 43:rd byte (for handshake response)
        // have zero MSB because of curve25519 pubkey, so xor it with random.
        if (packet->data[0] < 4 && size >= 48)
          packet->data[35 + packet->data[0] * 4] ^= packet->data[15];
      }
      if (packet->data[0] == 4) {
        predicted_key_in_ = Read32(packet->data + 4);
        predicted_serial_in_ = ReadLE64(packet->data + 8);
      }
      return packet;
    } else {
      // Optimization when the 16 first bytes are known and prefixed to the packet
      assert(packet->data >= packet->data_buf);
      packet->data -= 16, packet->size += 16;
      predicted_serial_in_++;
      WriteLE32(packet->data, 4);
      Write32(packet->data + 4, predicted_key_in_);
      WriteLE64(packet->data + 8, predicted_serial_in_);
      return packet;
    }
  }
  return NULL;
}

Packet *TcpPacketHandler::GetNextWireguardPacketNormal() {
  while (queue_.size() >= 2) {
    uint32 packet_header = queue_.PeekUint16();
    uint32 packet_size = packet_header & 0x7FFF;
    if (packet_size + 2 > kPacketCapacity) {
error:
      error_flag_ = true;
      return NULL;
    }
    if (packet_size + 2 > queue_.size())
      return NULL;
    Packet *packet = queue_.Read(packet_size + 2);
    if (!packet)
      goto error;
    if (!(packet_header & 0x8000)) {
      packet->data += 2, packet->size -= 2;
      if (packet->data[0] == 4 && packet->size >= 16) {
        predicted_key_in_ = Read32(packet->data + 4);
        predicted_serial_in_ = ReadLE64(packet->data + 8);
      }
    } else {
      // Optimization when the 16 first bytes are known and prefixed to the packet
      assert(packet->data >= packet->data_buf);
      packet->data -= 14, packet->size += 14;
      predicted_serial_in_++;
      WriteLE32(packet->data, 4);
      Write32(packet->data + 4, predicted_key_in_);
      WriteLE64(packet->data + 8, predicted_serial_in_);
    }
    return packet;
  }
  return NULL;
}

#define TLS_ASYNC_BEGIN() switch (tls_read_state_) {
#define TLS_ASYNC_RESUMEPOINT(label) tls_read_state_ = (label); case label:
#define TLS_ASYNC_WAIT(expr, label) case label: if (!(expr)) { tls_read_state_ = (label); return NULL; }
#define TLS_ASYNC_END() }

// Unwrap the TLS framing
Packet *TcpPacketHandler::GetNextWireguardPacketTLS13() {
  uint8 header[5];
  Packet *packet;

  enum {
    TLS_STATE_INIT = 0,
    TLS_WAIT_HANDSHAKE = 1,
    TLS_WAIT_DATA = 2,
    TLS_READ_PACKETS = 3,
    TLS_WAIT_JUNK = 4,
    TLS_ERROR = 5,
  };
  TLS_ASYNC_BEGIN();
  for(;;) {
    TLS_ASYNC_WAIT(queue_.size() >= 5, TLS_STATE_INIT);
    queue_.Read(header, 5);
    tls_bytes_left_ = ReadBE16(header + 3);
    if (header[0] == 23) {
      if (!decryptor_initialized_)
        goto error; // no key yet
      // Read the next |tls_bytes_left_| bytes and push them to the tls_queue_.
      while (tls_bytes_left_ != 0) {
        TLS_ASYNC_WAIT(queue_.size() != 0, TLS_WAIT_DATA);
        if (!(packet = queue_.ReadUpTo(tls_bytes_left_))) goto error;
        tls_bytes_left_ -= packet->size;
        tls_queue_.Add(packet);
        TLS_ASYNC_RESUMEPOINT(TLS_READ_PACKETS);
        if ((packet = GetNextWireguardPacketObfuscate(&tls_queue_)) != NULL)
          return packet;
      }
    } else {
      if (tls_bytes_left_ > kPacketCapacity)
        goto error;  // too large packet?
      if (header[0] == 22) {
        TLS_ASYNC_WAIT(tls_bytes_left_ <= queue_.size(), TLS_WAIT_HANDSHAKE);
        if (!(packet = queue_.Read(tls_bytes_left_)))
          goto error; // eom
        // Initialize decryptor
        if (!decryptor_initialized_ && packet->size >= 39 + 32) {
          // Store the session ID, so we can include it in server hello.
          memcpy(tls_session_id_, packet->data + 39, 32);
          // Initialize chacha decryptor
          SetChachaStreamingKey(&decryptor_, packet->data, packet->size);
          decryptor_initialized_ = true;
        }
        FreePacket(packet);
      } else if (header[0] == 20) {
        TLS_ASYNC_WAIT(tls_bytes_left_ <= queue_.size(), TLS_WAIT_JUNK);
        if (!(packet = queue_.Read(tls_bytes_left_)))
          goto error; // eom
        FreePacket(packet);
      } else {
error:
        TLS_ASYNC_RESUMEPOINT(TLS_ERROR);
        error_flag_ = true;
        return NULL;
      }
    }
  }
  TLS_ASYNC_END();
  return NULL;
}

void TcpPacketHandler::PrepareOutgoingPacketsWithHeader(Packet *p) {
  uint8 buf[1024];
  size_t hello_size;

  if (obfuscation_mode_ == kObfuscationMode_Encrypted) {
    // Ensure it doesn't look like a tls or a regular packet.
    do {
      OsGetRandomBytes(buf, CRYPTO_HEADER_SIZE);
    } while (ReadBE16(buf) == 0x1603 || ReadBE16(buf) <= 1500);
    
    SetChachaStreamingKey(&encryptor_, buf, CRYPTO_HEADER_SIZE);
    hello_size = CRYPTO_HEADER_SIZE;
  } else {
    hello_size = (write_state_ == 0) ? CreateTls13ClientHello(buf) : CreateTls13ServerHello(buf);
    // This could fail if the server tries to send a packet before the client sent hello.
    if (hello_size == ~(size_t)0) {
      RERROR("Trying to send server message before client hello");
      p->size = 0;
      return;
    }
  }
  write_state_ = 2;
  PrepareOutgoingPackets(p);
  if (hello_size + p->size > kPacketCapacity) {
    RERROR("Outgoing TCP packet too big.");
    return;
  }
  memmove(p->data_buf + hello_size, exch(p->data, p->data_buf), postinc(p->size, (uint)hello_size));
  memcpy(p->data_buf, buf, hello_size);
}


void TcpPacketHandler::PrepareOutgoingPackets(Packet *p) {
  if (obfuscation_mode_ == kObfuscationMode_None) {
    PrepareOutgoingPacketsNormal(p);
  } else {
    if (write_state_ != 2) {
      PrepareOutgoingPacketsWithHeader(p);
      return;
    }
    if (obfuscation_mode_ == kObfuscationMode_Encrypted)
      PrepareOutgoingPacketsObfuscate(p);
    else
      PrepareOutgoingPacketsTLS13(p);
  }
}

Packet *TcpPacketHandler::GetNextWireguardPacket() {
  // If this is an incoming connection, try to guess what type of obfuscation
  // we're using, if any.
  for (;;) {
    if (obfuscation_mode_ == kObfuscationMode_None)
      return GetNextWireguardPacketNormal();
    else if (obfuscation_mode_ == kObfuscationMode_Encrypted)
      return GetNextWireguardPacketObfuscate(&queue_);
    else if (obfuscation_mode_ != kObfuscationMode_Autodetect)
      return GetNextWireguardPacketTLS13();

    // Try and autodetect based on the first 2 bytes.
    if (queue_.size() < 2)
      return NULL;

    uint16 header = queue_.PeekUint16();
    if (header == 0x1603) {
      // This is a SSL client hello, but don't know if it's
      // chrome or ff, so use ff.
      obfuscation_mode_ = kObfuscationMode_TlsFirefox;
    } else if (header <= 1500) {
      // Unobfuscated wireguard headers always start with a low value.
      obfuscation_mode_ = kObfuscationMode_None;
    } else {
      read_state_ = READ_CRYPTO_HEADER;
      obfuscation_mode_ = kObfuscationMode_Encrypted;
    }
  }
}


#if defined(OS_WIN) || defined(USE_MULTITHREADED_NETWORKING)
void SimplePacketPool::FreeSomePacketsInner() {
  int n = freed_packets_count_ - 24;
  Packet **p = &freed_packets_;
  for (; n; n--)
    p = &Packet_NEXT(*p);
  FreePackets(exch(freed_packets_, *p), p, exch(freed_packets_count_, 24) - 24);
}
#endif


