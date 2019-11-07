/* Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 *
 * This file is provided under a dual BSD/GPLv2 license.
 *
 * SipHash: a fast short-input PRF
 * https://131002.net/siphash/
 *
 * This implementation is specifically for SipHash2-4 for a secure PRF
 * and HalfSipHash1-3/SipHash1-3 for an insecure PRF only suitable for
 * hashtables.
 */
#include "stdafx.h"

#include "crypto/siphash/siphash.h"
#include "tunsafe_endian.h"

#define SIPROUND \
  do { \
  v0 += v1; v1 = rol64(v1, 13); v1 ^= v0; v0 = rol64(v0, 32); \
  v2 += v3; v3 = rol64(v3, 16); v3 ^= v2; \
  v0 += v3; v3 = rol64(v3, 21); v3 ^= v0; \
  v2 += v1; v1 = rol64(v1, 17); v1 ^= v2; v2 = rol64(v2, 32); \
  } while (0)

#define PREAMBLE(len) \
  uint64 v0 = 0x736f6d6570736575ULL; \
  uint64 v1 = 0x646f72616e646f6dULL; \
  uint64 v2 = 0x6c7967656e657261ULL; \
  uint64 v3 = 0x7465646279746573ULL; \
  uint64 b = ((uint64)(len)) << 56; \
  v3 ^= key->key[1]; \
  v2 ^= key->key[0]; \
  v1 ^= key->key[1]; \
  v0 ^= key->key[0];

#define POSTAMBLE24 \
  v3 ^= b; \
  SIPROUND; \
  SIPROUND; \
  v0 ^= b; \
  v2 ^= 0xff; \
  SIPROUND; \
  SIPROUND; \
  SIPROUND; \
  SIPROUND; \
  return (v0 ^ v1) ^ (v2 ^ v3);

#define POSTAMBLE13 \
  v3 ^= b; \
  SIPROUND; \
  v0 ^= b; \
  v2 ^= 0xff; \
  SIPROUND; \
  SIPROUND; \
  SIPROUND; \
  return (v0 ^ v1) ^ (v2 ^ v3);


uint64 siphash(const void *data, size_t len, const siphash_key_t *key) {
  const uint8 *end = (uint8*)data + len - (len % sizeof(uint64));
  const uint8 left = len & (sizeof(uint64) - 1);
  uint64 m;
  PREAMBLE(len)
  for (; data != end; data = (uint8*)data + sizeof(uint64)) {
    m = ReadLE64(data);
    v3 ^= m;
    SIPROUND;
    SIPROUND;
    v0 ^= m;
  }
  switch (left) {
  case 7: b |= ((uint64)end[6]) << 48;
  case 6: b |= ((uint64)end[5]) << 40;
  case 5: b |= ((uint64)end[4]) << 32;
  case 4: b |= ReadLE32(data); break;
  case 3: b |= ((uint64)end[2]) << 16;
  case 2: b |= ReadLE16(data); break;
  case 1: b |= end[0];
  }
  POSTAMBLE24
}

/**
 * siphash_1u64 - compute 64-bit siphash PRF value of a uint64
 * @first: first uint64
 * @key: the siphash key
 */
uint64 siphash_1u64(const uint64 first, const siphash_key_t *key)
{
  PREAMBLE(8)
  v3 ^= first;
  SIPROUND;
  SIPROUND;
  v0 ^= first;
  POSTAMBLE24
}

/**
 * siphash_2u64 - compute 64-bit siphash PRF value of 2 uint64
 * @first: first uint64
 * @second: second uint64
 * @key: the siphash key
 */
uint64 siphash_2u64(const uint64 first, const uint64 second, const siphash_key_t *key)
{
  PREAMBLE(16)
  v3 ^= first;
  SIPROUND;
  SIPROUND;
  v0 ^= first;
  v3 ^= second;
  SIPROUND;
  SIPROUND;
  v0 ^= second;
  POSTAMBLE24
}

/**
 * siphash_3u64 - compute 64-bit siphash PRF value of 3 uint64
 * @first: first uint64
 * @second: second uint64
 * @third: third uint64
 * @key: the siphash key
 */
uint64 siphash_3u64(const uint64 first, const uint64 second, const uint64 third,
     const siphash_key_t *key)
{
  PREAMBLE(24)
  v3 ^= first;
  SIPROUND;
  SIPROUND;
  v0 ^= first;
  v3 ^= second;
  SIPROUND;
  SIPROUND;
  v0 ^= second;
  v3 ^= third;
  SIPROUND;
  SIPROUND;
  v0 ^= third;
  POSTAMBLE24
}

/**
* siphash13_3u64 - compute 64-bit siphash13 PRF value of 3 uint64
* @first: first uint64
* @second: second uint64
* @third: third uint64
* @key: the siphash key
*/
uint64 siphash13_3u64(const uint64 first, const uint64 second, const uint64 third,
                      const siphash_key_t *key) {
  PREAMBLE(24)
  v3 ^= first;
  SIPROUND;
  v0 ^= first;
  v3 ^= second;
  SIPROUND;
  v0 ^= second;
  v3 ^= third;
  SIPROUND;
  v0 ^= third;
  POSTAMBLE13
}

uint64 siphash13_2u64(const uint64 first, const uint64 second, const siphash_key_t *key) {
  PREAMBLE(24)
  v3 ^= first;
  SIPROUND;
  v0 ^= first;
  v3 ^= second;
  SIPROUND;
  v0 ^= second;
  POSTAMBLE13
}

uint64 siphash13_4u64(const uint64 first, const uint64 second, const uint64 third, const uint64 fourth,
                      const siphash_key_t *key) {
  PREAMBLE(24)
  v3 ^= first;
  SIPROUND;
  v0 ^= first;
  v3 ^= second;
  SIPROUND;
  v0 ^= second;
  v3 ^= third;
  SIPROUND;
  v0 ^= third;
  v3 ^= fourth;
  SIPROUND;
  v0 ^= fourth;
  POSTAMBLE13
}

/**
 * siphash_4u64 - compute 64-bit siphash PRF value of 4 uint64
 * @first: first uint64
 * @second: second uint64
 * @third: third uint64
 * @forth: forth uint64
 * @key: the siphash key
 */
uint64 siphash_4u64(const uint64 first, const uint64 second, const uint64 third,
     const uint64 forth, const siphash_key_t *key)
{
  PREAMBLE(32)
  v3 ^= first;
  SIPROUND;
  SIPROUND;
  v0 ^= first;
  v3 ^= second;
  SIPROUND;
  SIPROUND;
  v0 ^= second;
  v3 ^= third;
  SIPROUND;
  SIPROUND;
  v0 ^= third;
  v3 ^= forth;
  SIPROUND;
  SIPROUND;
  v0 ^= forth;
  POSTAMBLE24
}

uint64 siphash_1u32(const uint32 first, const siphash_key_t *key)
{
  PREAMBLE(4)
  b |= first;
  POSTAMBLE24
}

uint64 siphash_3u32(const uint32 first, const uint32 second, const uint32 third,
     const siphash_key_t *key)
{
  uint64 combined = (uint64)second << 32 | first;
  PREAMBLE(12)
  v3 ^= combined;
  SIPROUND;
  SIPROUND;
  v0 ^= combined;
  b |= third;
  POSTAMBLE24
}

uint64 siphash_u64_u32(const uint64 combined, const uint32 third, const siphash_key_t *key) {
  PREAMBLE(12)
  v3 ^= combined;
  SIPROUND;
  SIPROUND;
  v0 ^= combined;
  b |= third;
  POSTAMBLE24
}

