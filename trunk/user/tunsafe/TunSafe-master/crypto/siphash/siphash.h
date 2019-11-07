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

#ifndef TUNSAFE_CRYPTO_SIPHASH_H_
#define TUNSAFE_CRYPTO_SIPHASH_H_

#include "tunsafe_types.h"

typedef struct {
	uint64 key[2];
} siphash_key_t;

uint64 siphash_1u64(const uint64 a, const siphash_key_t *key);
uint64 siphash_2u64(const uint64 a, const uint64 b, const siphash_key_t *key);
uint64 siphash_3u64(const uint64 a, const uint64 b, const uint64 c,
		 const siphash_key_t *key);
uint64 siphash_4u64(const uint64 a, const uint64 b, const uint64 c, const uint64 d,
		 const siphash_key_t *key);
uint64 siphash_1u32(const uint32 a, const siphash_key_t *key);
uint64 siphash_3u32(const uint32 a, const uint32 b, const uint32 c,
		 const siphash_key_t *key);

static inline uint64 siphash_2u32(const uint32 a, const uint32 b,
			       const siphash_key_t *key)
{
	return siphash_1u64((uint64)b << 32 | a, key);
}
static inline uint64 siphash_4u32(const uint32 a, const uint32 b, const uint32 c,
			       const uint32 d, const siphash_key_t *key)
{
	return siphash_2u64((uint64)b << 32 | a, (uint64)d << 32 | c, key);
}

uint64 siphash_u64_u32(const uint64 combined, const uint32 third, const siphash_key_t *key);

/**
 * siphash - compute 64-bit siphash PRF value
 * @data: buffer to hash
 * @size: size of @data
 * @key: the siphash key
 */
uint64 siphash(const void *data, size_t len, const siphash_key_t *key);

uint64 siphash13_2u64(const uint64 first, const uint64 second, const siphash_key_t *key);
uint64 siphash13_3u64(const uint64 first, const uint64 second, const uint64 third,
                      const siphash_key_t *key);

uint64 siphash13_4u64(const uint64 first, const uint64 second, const uint64 third,
                      const uint64 fourth, const siphash_key_t *key);

#endif  // TUNSAFE_CRYPTO_SIPHASH_H_
