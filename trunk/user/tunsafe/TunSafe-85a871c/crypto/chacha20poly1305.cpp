/* SPDX-License-Identifier: OpenSSL OR (BSD-3-Clause OR GPL-2.0)
 *
 * Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 * Copyright 2016 The OpenSSL Project Authors. All Rights Reserved.
 */

#include "stdafx.h"
#include "crypto/chacha20poly1305.h"
#include "tunsafe_types.h"
#include "tunsafe_endian.h"
#include "build_config.h"
#include "tunsafe_cpu.h"
#include "crypto_ops.h"
#include <string.h>
#include <assert.h>
#include "util.h"

enum {
	CHACHA20_IV_SIZE = 16,
	CHACHA20_KEY_SIZE = 32,
	CHACHA20_BLOCK_SIZE = 64,
	POLY1305_BLOCK_SIZE = 16,
	POLY1305_KEY_SIZE = 32,
	POLY1305_MAC_SIZE = 16
};


#if defined(OS_MACOSX) || !WITH_AVX512_OPTIMIZATIONS
#define CHACHA20_WITH_AVX512 0
#else
#define CHACHA20_WITH_AVX512 1
#endif

#ifndef CHACHA20_WITH_ASM
#define CHACHA20_WITH_ASM 1
#endif  // CHACHA20_WITH_ASM


extern "C" {
void _cdecl hchacha20_ssse3(uint8 *derived_key, const uint8 *nonce, const uint8 *key);
void _cdecl chacha20_ssse3(uint8 *out, const uint8 *in, size_t len, const uint32 key[8], const uint32 counter[4]);
void _cdecl chacha20_avx2(uint8 *out, const uint8 *in, size_t len, const uint32 key[8], const uint32 counter[4]);
void _cdecl chacha20_avx512(uint8 *out, const uint8 *in, size_t len, const uint32 key[8], const uint32 counter[4]);
void _cdecl chacha20_avx512vl(uint8 *out, const uint8 *in, size_t len, const uint32 key[8], const uint32 counter[4]);
void _cdecl poly1305_init_x86_64(void *ctx, const uint8 key[16]);
void _cdecl poly1305_blocks_x86_64(void *ctx, const uint8 *inp, size_t len, uint32 padbit);
void _cdecl poly1305_emit_x86_64(void *ctx, uint8 mac[16], const uint32 nonce[4]);
void _cdecl poly1305_emit_avx(void *ctx, uint8 mac[16], const uint32 nonce[4]);
void _cdecl poly1305_blocks_avx(void *ctx, const uint8 *inp, size_t len, uint32 padbit);
void _cdecl poly1305_blocks_avx2(void *ctx, const uint8 *inp, size_t len, uint32 padbit);
void _cdecl poly1305_blocks_avx512(void *ctx, const uint8 *inp, size_t len, uint32 padbit);

#if defined(ARCH_CPU_ARM_FAMILY)
void chacha20_arm(uint8 *out, const uint8 *in, size_t len, const uint32 key[8], const uint32 counter[4]);
void chacha20_neon(uint8 *out, const uint8 *in, size_t len, const uint32 key[8], const uint32 counter[4]);
#endif
void poly1305_init_arm(void *ctx, const uint8 key[16]);
void poly1305_blocks_arm(void *ctx, const uint8 *inp, size_t len, uint32 padbit);
void poly1305_emit_arm(void *ctx, uint8 mac[16], const uint32 nonce[4]);
void poly1305_blocks_neon(void *ctx, const uint8 *inp, size_t len, uint32 padbit);
void poly1305_emit_neon(void *ctx, uint8 mac[16], const uint32 nonce[4]);

}

struct chacha20_ctx {
	uint32 state[CHACHA20_BLOCK_SIZE / sizeof(uint32)];
};

void crypto_xor(uint8 *dst, const uint8 *src, size_t n) {
  for (; n >= 4; n -= 4, dst += 4, src += 4)
    *(uint32*)dst ^= *(uint32*)src;
  for (; n; n--)
    *dst++ ^= *src++;
}

int memcmp_crypto(const uint8 *a, const uint8 *b, size_t n) {
  int rv = 0;
  for (; n >= 4; n -= 4, a += 4, b += 4)
    rv |= *(uint32*)a ^ *(uint32*)b;
  for (; n; n--)
    rv |= *a++ ^ *b++;
  return rv;
}

#define QUARTER_ROUND(x, a, b, c, d) ( \
	x[a] += x[b], \
	x[d] = rol32((x[d] ^ x[a]), 16), \
	x[c] += x[d], \
	x[b] = rol32((x[b] ^ x[c]), 12), \
	x[a] += x[b], \
	x[d] = rol32((x[d] ^ x[a]), 8), \
	x[c] += x[d], \
	x[b] = rol32((x[b] ^ x[c]), 7) \
)

#define C(i, j) (i * 4 + j)

#define DOUBLE_ROUND(x) ( \
	/* Column Round */ \
	QUARTER_ROUND(x, C(0, 0), C(1, 0), C(2, 0), C(3, 0)), \
	QUARTER_ROUND(x, C(0, 1), C(1, 1), C(2, 1), C(3, 1)), \
	QUARTER_ROUND(x, C(0, 2), C(1, 2), C(2, 2), C(3, 2)), \
	QUARTER_ROUND(x, C(0, 3), C(1, 3), C(2, 3), C(3, 3)), \
	/* Diagonal Round */ \
	QUARTER_ROUND(x, C(0, 0), C(1, 1), C(2, 2), C(3, 3)), \
	QUARTER_ROUND(x, C(0, 1), C(1, 2), C(2, 3), C(3, 0)), \
	QUARTER_ROUND(x, C(0, 2), C(1, 3), C(2, 0), C(3, 1)), \
	QUARTER_ROUND(x, C(0, 3), C(1, 0), C(2, 1), C(3, 2)) \
)

#define TWENTY_ROUNDS(x) ( \
	DOUBLE_ROUND(x), \
	DOUBLE_ROUND(x), \
	DOUBLE_ROUND(x), \
	DOUBLE_ROUND(x), \
	DOUBLE_ROUND(x), \
	DOUBLE_ROUND(x), \
	DOUBLE_ROUND(x), \
	DOUBLE_ROUND(x), \
	DOUBLE_ROUND(x), \
	DOUBLE_ROUND(x) \
)

SAFEBUFFERS static void chacha20_block_generic(struct chacha20_ctx *ctx, uint32 *stream)
{
	uint32 x[CHACHA20_BLOCK_SIZE / sizeof(uint32)];
	int i;

	for (i = 0; i < ARRAY_SIZE(x); ++i)
		x[i] = ctx->state[i];

	TWENTY_ROUNDS(x);

	for (i = 0; i < ARRAY_SIZE(x); ++i)
		stream[i] = ToLE32(x[i] + ctx->state[i]);

	++ctx->state[12];
}

SAFEBUFFERS static void hchacha20_generic(uint8 derived_key[CHACHA20POLY1305_KEYLEN], const uint8 nonce[16], const uint8 key[CHACHA20POLY1305_KEYLEN])
{
	uint32 *out = (uint32 *)derived_key;
	uint32 x[] = {
		0x61707865, 0x3320646e, 0x79622d32, 0x6b206574,
    ReadLE32(key + 0), ReadLE32(key + 4), ReadLE32(key + 8), ReadLE32(key + 12),
    ReadLE32(key + 16), ReadLE32(key + 20), ReadLE32(key + 24), ReadLE32(key + 28),
    ReadLE32(nonce +  0), ReadLE32(nonce +  4), ReadLE32(nonce +  8), ReadLE32(nonce + 12)
	};

	TWENTY_ROUNDS(x);

	out[0] = ToLE32(x[0]);
	out[1] = ToLE32(x[1]);
	out[2] = ToLE32(x[2]);
	out[3] = ToLE32(x[3]);
	out[4] = ToLE32(x[12]);
	out[5] = ToLE32(x[13]);
	out[6] = ToLE32(x[14]);
	out[7] = ToLE32(x[15]);
}

static inline void hchacha20(uint8 derived_key[CHACHA20POLY1305_KEYLEN], const uint8 nonce[16], const uint8 key[CHACHA20POLY1305_KEYLEN])
{
#if defined(ARCH_CPU_X86_64) && defined(COMPILER_MSVC) && CHACHA20_WITH_ASM
	if (X86_PCAP_SSSE3) {
		hchacha20_ssse3(derived_key, nonce, key);
		return;
	}
#endif  // defined(ARCH_CPU_X86_64)
	hchacha20_generic(derived_key, nonce, key);
}

#define chacha20_initial_state(key, nonce) {{ \
	0x61707865, 0x3320646e, 0x79622d32, 0x6b206574, \
	ReadLE32((key) + 0), ReadLE32((key) + 4), ReadLE32((key) + 8), ReadLE32((key) + 12), \
	ReadLE32((key) + 16), ReadLE32((key) + 20), ReadLE32((key) + 24), ReadLE32((key) + 28), \
	0, 0, ReadLE32((nonce) +  0), ReadLE32((nonce) + 4) \
}}

SAFEBUFFERS static void chacha20_crypt(struct chacha20_ctx *ctx, uint8 *dst, const uint8 *src, uint32 bytes)
{
	uint32 buf[CHACHA20_BLOCK_SIZE / sizeof(uint32)];

  if (bytes == 0)
    return;

#if defined(ARCH_CPU_X86_64) && CHACHA20_WITH_ASM
#if CHACHA20_WITH_AVX512
	if (X86_PCAP_AVX512F) {
		chacha20_avx512(dst, src, bytes, &ctx->state[4], &ctx->state[12]);
		ctx->state[12] += (bytes + 63) / 64;
		return;
	}
	if (X86_PCAP_AVX512VL) {
		chacha20_avx512vl(dst, src, bytes, &ctx->state[4], &ctx->state[12]);
		ctx->state[12] += (bytes + 63) / 64;
		return;
	}
#endif  // CHACHA20_WITH_AVX512
  if (X86_PCAP_AVX2) {
    chacha20_avx2(dst, src, bytes, &ctx->state[4], &ctx->state[12]);
    ctx->state[12] += (bytes + 63) / 64;
    return;
  }
  if (X86_PCAP_SSSE3) {
    assert(bytes);
    chacha20_ssse3(dst, src, bytes, &ctx->state[4], &ctx->state[12]);
    ctx->state[12] += (bytes + 63) / 64;
    return;
  }
#endif  // defined(ARCH_CPU_X86_64)

#if defined(ARCH_CPU_ARM_FAMILY) && CHACHA20_WITH_ASM
  if (ARM_PCAP_NEON) {
    chacha20_neon(dst, src, bytes, &ctx->state[4], &ctx->state[12]);
  } else {
    chacha20_arm(dst, src, bytes, &ctx->state[4], &ctx->state[12]);
  }
  ctx->state[12] += (bytes + 63) / 64;
  return;
#endif  // defined(ARCH_CPU_ARM_FAMILY)


	if (dst != src)
		memcpy(dst, src, bytes);

	while (bytes >= CHACHA20_BLOCK_SIZE) {
		chacha20_block_generic(ctx, buf);
		crypto_xor(dst, (uint8 *)buf, CHACHA20_BLOCK_SIZE);
		bytes -= CHACHA20_BLOCK_SIZE;
		dst += CHACHA20_BLOCK_SIZE;
	}
	if (bytes) {
		chacha20_block_generic(ctx, buf);
		crypto_xor(dst, (uint8 *)buf, bytes);
	}
}

struct poly1305_ctx {
	uint8 opaque[24 * sizeof(uint64)];
	uint32 nonce[4];
	uint8 data[POLY1305_BLOCK_SIZE];
	size_t num;
};

#if !(defined(CONFIG_X86_64) || defined(CONFIG_ARM) || defined(CONFIG_ARM64) || (defined(CONFIG_MIPS) && defined(CONFIG_64BIT))) || !CHACHA20_WITH_ASM
struct poly1305_internal {
	uint32 h[5];
	uint32 r[4];
};

static void poly1305_init_generic(void *ctx, const uint8 key[16]) {
	struct poly1305_internal *st = (struct poly1305_internal *)ctx;

	/* h = 0 */
	st->h[0] = 0;
	st->h[1] = 0;
	st->h[2] = 0;
	st->h[3] = 0;
	st->h[4] = 0;

	/* r &= 0xffffffc0ffffffc0ffffffc0fffffff */
	st->r[0] = ReadLE32(&key[ 0]) & 0x0fffffff;
	st->r[1] = ReadLE32(&key[ 4]) & 0x0ffffffc;
	st->r[2] = ReadLE32(&key[ 8]) & 0x0ffffffc;
	st->r[3] = ReadLE32(&key[12]) & 0x0ffffffc;
}

static void poly1305_blocks_generic(void *ctx, const uint8 *inp, size_t len, uint32 padbit)
{
#define CONSTANT_TIME_CARRY(a,b) ((a ^ ((a ^ b) | ((a - b) ^ b))) >> (sizeof(a) * 8 - 1))
	struct poly1305_internal *st = (struct poly1305_internal *)ctx;
	uint32 r0, r1, r2, r3;
	uint32 s1, s2, s3;
	uint32 h0, h1, h2, h3, h4, c;
	uint64 d0, d1, d2, d3;

	r0 = st->r[0];
	r1 = st->r[1];
	r2 = st->r[2];
	r3 = st->r[3];

	s1 = r1 + (r1 >> 2);
	s2 = r2 + (r2 >> 2);
	s3 = r3 + (r3 >> 2);

	h0 = st->h[0];
	h1 = st->h[1];
	h2 = st->h[2];
	h3 = st->h[3];
	h4 = st->h[4];

	while (len >= POLY1305_BLOCK_SIZE) {
		/* h += m[i] */
		h0 = (uint32)(d0 = (uint64)h0 + ReadLE32(inp + 0));
		h1 = (uint32)(d1 = (uint64)h1 + (d0 >> 32) + ReadLE32(inp + 4));
		h2 = (uint32)(d2 = (uint64)h2 + (d1 >> 32) + ReadLE32(inp + 8));
		h3 = (uint32)(d3 = (uint64)h3 + (d2 >> 32) + ReadLE32(inp + 12));
		h4 += (uint32)(d3 >> 32) + padbit;

		/* h *= r "%" p, where "%" stands for "partial remainder" */
		d0 = ((uint64)h0 * r0) +
		     ((uint64)h1 * s3) +
		     ((uint64)h2 * s2) +
		     ((uint64)h3 * s1);
		d1 = ((uint64)h0 * r1) +
		     ((uint64)h1 * r0) +
		     ((uint64)h2 * s3) +
		     ((uint64)h3 * s2) +
		     (h4 * s1);
		d2 = ((uint64)h0 * r2) +
		     ((uint64)h1 * r1) +
		     ((uint64)h2 * r0) +
		     ((uint64)h3 * s3) +
		     (h4 * s2);
		d3 = ((uint64)h0 * r3) +
		     ((uint64)h1 * r2) +
		     ((uint64)h2 * r1) +
		     ((uint64)h3 * r0) +
		     (h4 * s3);
		h4 = (h4 * r0);

		/* last reduction step: */
		/* a) h4:h0 = h4<<128 + d3<<96 + d2<<64 + d1<<32 + d0 */
		h0 = (uint32)d0;
		h1 = (uint32)(d1 += d0 >> 32);
		h2 = (uint32)(d2 += d1 >> 32);
		h3 = (uint32)(d3 += d2 >> 32);
		h4 += (uint32)(d3 >> 32);
		/* b) (h4:h0 += (h4:h0>>130) * 5) %= 2^130 */
		c = (h4 >> 2) + (h4 & ~3U);
		h4 &= 3;
		h0 += c;
		h1 += (c = CONSTANT_TIME_CARRY(h0,c));
		h2 += (c = CONSTANT_TIME_CARRY(h1,c));
		h3 += (c = CONSTANT_TIME_CARRY(h2,c));
		h4 += CONSTANT_TIME_CARRY(h3,c);
		/*
		 * Occasional overflows to 3rd bit of h4 are taken care of
		 * "naturally". If after this point we end up at the top of
		 * this loop, then the overflow bit will be accounted for
		 * in next iteration. If we end up in poly1305_emit, then
		 * comparison to modulus below will still count as "carry
		 * into 131st bit", so that properly reduced value will be
		 * picked in conditional move.
		 */

		inp += POLY1305_BLOCK_SIZE;
		len -= POLY1305_BLOCK_SIZE;
	}

	st->h[0] = h0;
	st->h[1] = h1;
	st->h[2] = h2;
	st->h[3] = h3;
	st->h[4] = h4;
#undef CONSTANT_TIME_CARRY
}

static void poly1305_emit_generic(void *ctx, uint8 mac[16], const uint32 nonce[4])
{
	struct poly1305_internal *st = (struct poly1305_internal *)ctx;
	uint32 *omac = (uint32 *)mac;
	uint32 h0, h1, h2, h3, h4;
	uint32 g0, g1, g2, g3, g4;
	uint64 t;
	uint32 mask;

	h0 = st->h[0];
	h1 = st->h[1];
	h2 = st->h[2];
	h3 = st->h[3];
	h4 = st->h[4];

	/* compare to modulus by computing h + -p */
	g0 = (uint32)(t = (uint64)h0 + 5);
	g1 = (uint32)(t = (uint64)h1 + (t >> 32));
	g2 = (uint32)(t = (uint64)h2 + (t >> 32));
	g3 = (uint32)(t = (uint64)h3 + (t >> 32));
	g4 = h4 + (uint32)(t >> 32);

	/* if there was carry into 131st bit, h3:h0 = g3:g0 */
	mask = 0 - (g4 >> 2);
	g0 &= mask;
	g1 &= mask;
	g2 &= mask;
	g3 &= mask;
	mask = ~mask;
	h0 = (h0 & mask) | g0;
	h1 = (h1 & mask) | g1;
	h2 = (h2 & mask) | g2;
	h3 = (h3 & mask) | g3;

	/* mac = (h + nonce) % (2^128) */
	h0 = (uint32)(t = (uint64)h0 + nonce[0]);
	h1 = (uint32)(t = (uint64)h1 + (t >> 32) + nonce[1]);
	h2 = (uint32)(t = (uint64)h2 + (t >> 32) + nonce[2]);
	h3 = (uint32)(t = (uint64)h3 + (t >> 32) + nonce[3]);

	omac[0] = ToLE32(h0);
	omac[1] = ToLE32(h1);
	omac[2] = ToLE32(h2);
	omac[3] = ToLE32(h3);
}
#endif

SAFEBUFFERS static void poly1305_init(struct poly1305_ctx *ctx, const uint8 key[POLY1305_KEY_SIZE])
{
	ctx->nonce[0] = ReadLE32(&key[16]);
	ctx->nonce[1] = ReadLE32(&key[20]);
	ctx->nonce[2] = ReadLE32(&key[24]);
	ctx->nonce[3] = ReadLE32(&key[28]);

#if defined(ARCH_CPU_X86_64) && CHACHA20_WITH_ASM
	poly1305_init_x86_64(ctx->opaque, key);
#elif defined(ARCH_CPU_ARM_FAMILY) && CHACHA20_WITH_ASM
	poly1305_init_arm(ctx->opaque, key);
#elif defined(CONFIG_MIPS) && defined(CONFIG_64BIT)
	poly1305_init_mips(ctx->opaque, key);
#else
	poly1305_init_generic(ctx->opaque, key);
#endif
	ctx->num = 0;
}

static inline void poly1305_blocks(void *ctx, const uint8 *inp, size_t len, uint32 padbit)
{
#if defined(ARCH_CPU_X86_64) && CHACHA20_WITH_ASM
#if CHACHA20_WITH_AVX512
	if(X86_PCAP_AVX512F)
		poly1305_blocks_avx512(ctx, inp, len, padbit);
	else 
#endif  // CHACHA20_WITH_AVX512
  if (X86_PCAP_AVX2)
    poly1305_blocks_avx2(ctx, inp, len, padbit);
  else if (X86_PCAP_AVX)
    poly1305_blocks_avx(ctx, inp, len, padbit);
  else
		poly1305_blocks_x86_64(ctx, inp, len, padbit);
#elif defined(ARCH_CPU_ARM_FAMILY) && CHACHA20_WITH_ASM
  if (ARM_PCAP_NEON)
    poly1305_blocks_neon(ctx, inp, len, padbit);
  else
    poly1305_blocks_arm(ctx, inp, len, padbit);
#else
  poly1305_blocks_generic(ctx, inp, len, padbit);
#endif  // defined(ARCH_CPU_X86_64)
}

static inline void poly1305_emit(void *ctx, uint8 mac[16], const uint32 nonce[4])
{
#if defined(ARCH_CPU_X86_64) && CHACHA20_WITH_ASM
  if (X86_PCAP_AVX)
    poly1305_emit_avx(ctx, mac, nonce);
  else
    poly1305_emit_x86_64(ctx, mac, nonce);
#elif defined(ARCH_CPU_ARM_FAMILY) && CHACHA20_WITH_ASM
  if (ARM_PCAP_NEON)
    poly1305_emit_neon(ctx, mac, nonce);
  else
    poly1305_emit_arm(ctx, mac, nonce);
#else  // defined(ARCH_CPU_X86_64)
	poly1305_emit_generic(ctx, mac, nonce);
#endif  // defined(ARCH_CPU_X86_64)
} 

SAFEBUFFERS static void poly1305_update(struct poly1305_ctx *ctx, const uint8 *inp, size_t len)
{
	const size_t num = ctx->num;
	size_t rem;

	if (num) {
		rem = POLY1305_BLOCK_SIZE - num;
		if (len >= rem) {
			memcpy(ctx->data + num, inp, rem);
			poly1305_blocks(ctx->opaque, ctx->data, POLY1305_BLOCK_SIZE, 1);
			inp += rem;
			len -= rem;
		} else {
			/* Still not enough data to process a block. */
			memcpy(ctx->data + num, inp, len);
			ctx->num = num + len;
			return;
		}
	}

	rem = len % POLY1305_BLOCK_SIZE;
	len -= rem;

	if (len >= POLY1305_BLOCK_SIZE) {
		poly1305_blocks(ctx->opaque, inp, len, 1);
		inp += len;
	}

	if (rem)
		memcpy(ctx->data, inp, rem);

	ctx->num = rem;
}

SAFEBUFFERS static void poly1305_finish(struct poly1305_ctx *ctx, uint8 mac[16])
{
	size_t num = ctx->num;

	if (num) {
		ctx->data[num++] = 1;   /* pad bit */
		while (num < POLY1305_BLOCK_SIZE)
			ctx->data[num++] = 0;
		poly1305_blocks(ctx->opaque, ctx->data, POLY1305_BLOCK_SIZE, 0);
	}

	poly1305_emit(ctx->opaque, mac, ctx->nonce);

	/* zero out the state */
	memzero_crypto(ctx, sizeof(*ctx));
}

static const uint8 pad0[16] = { 0 };

SAFEBUFFERS static FORCEINLINE void poly1305_getmac(const uint8 *ad, size_t ad_len, const uint8 *src, size_t src_len, const uint8 key[POLY1305_KEY_SIZE], uint8 mac[CHACHA20POLY1305_AUTHTAGLEN]) {
  uint64 len[2];
  struct poly1305_ctx poly1305_state;

  poly1305_init(&poly1305_state, key);
  poly1305_update(&poly1305_state, ad, ad_len);
  poly1305_update(&poly1305_state, pad0, (0 - ad_len) & 0xf);
  poly1305_update(&poly1305_state, src, src_len);
  poly1305_update(&poly1305_state, pad0, (0 - src_len) & 0xf);
  len[0] = ToLE64(ad_len);
  len[1] = ToLE64(src_len);
  poly1305_update(&poly1305_state, (uint8 *)&len, sizeof(len));
  poly1305_finish(&poly1305_state, mac);
}

struct ChaChaState {
  struct chacha20_ctx chacha20_state;
  uint8 block0[CHACHA20_BLOCK_SIZE];
};

static inline void InitializeChaChaState(ChaChaState *st, const uint8 key[CHACHA20POLY1305_KEYLEN], uint64 nonce) {
  uint64 le_nonce = ToLE64(nonce);
  WriteLE64((uint8*)st, 0x3320646e61707865);
  WriteLE64((uint8*)st + 8, 0x6b20657479622d32);
  Write64((uint8*)st + 16, Read64(key + 0));
  Write64((uint8*)st + 24, Read64(key + 8));
  Write64((uint8*)st + 32, Read64(key + 16));
  Write64((uint8*)st + 40, Read64(key + 24));
  Write64((uint8*)st + 48, 0);
  Write64((uint8*)st + 56, Read64((uint8*)&le_nonce));

  Write64((uint8*)st + 64 + 0 * 8, 0);
  Write64((uint8*)st + 64 + 1 * 8, 0);
  Write64((uint8*)st + 64 + 2 * 8, 0);
  Write64((uint8*)st + 64 + 3 * 8, 0);
  Write64((uint8*)st + 64 + 4 * 8, 0);
  Write64((uint8*)st + 64 + 5 * 8, 0);
  Write64((uint8*)st + 64 + 6 * 8, 0);
  Write64((uint8*)st + 64 + 7 * 8, 0);
}

SAFEBUFFERS void poly1305_get_mac(const uint8 *src, size_t src_len,
                     const uint8 *ad, const size_t ad_len,
                     const uint64 nonce, const uint8 key[CHACHA20POLY1305_KEYLEN],
                     uint8 mac[CHACHA20POLY1305_AUTHTAGLEN]) {
  ChaChaState st;

  InitializeChaChaState(&st, key, nonce);
  chacha20_crypt(&st.chacha20_state, st.block0, st.block0, sizeof(st.block0));
  poly1305_getmac(ad, ad_len, src, src_len, st.block0, mac);
  memzero_crypto(&st, sizeof(st));
}

SAFEBUFFERS void chacha20poly1305_encrypt(uint8 *dst, const uint8 *src, const size_t src_len,
					      const uint8 *ad, const size_t ad_len,
					      const uint64 nonce, const uint8 key[CHACHA20POLY1305_KEYLEN]) {
  ChaChaState st;

  InitializeChaChaState(&st, key, nonce);
	chacha20_crypt(&st.chacha20_state, st.block0, st.block0, sizeof(st.block0));
  chacha20_crypt(&st.chacha20_state, dst, src, (uint32)src_len);
  poly1305_getmac(ad, ad_len, dst, src_len, st.block0, dst + src_len);
  memzero_crypto(&st, sizeof(st));
}

SAFEBUFFERS void chacha20poly1305_decrypt_get_mac(uint8 *dst, const uint8 *src, const size_t src_len,
                                      const uint8 *ad, const size_t ad_len,
                                      const uint64 nonce, const uint8 key[CHACHA20POLY1305_KEYLEN],
                                      uint8 mac[CHACHA20POLY1305_AUTHTAGLEN]) {
  ChaChaState st;

  InitializeChaChaState(&st, key, nonce);
  chacha20_crypt(&st.chacha20_state, st.block0, st.block0, sizeof(st.block0));
  poly1305_getmac(ad, ad_len, src, src_len, st.block0, mac);
  chacha20_crypt(&st.chacha20_state, dst, src, (uint32)src_len);
  memzero_crypto(&st, sizeof(st));
}

SAFEBUFFERS bool chacha20poly1305_decrypt(uint8 *dst, const uint8 *src, const size_t src_len,
                              const uint8 *ad, const size_t ad_len,
                              const uint64 nonce, const uint8 key[CHACHA20POLY1305_KEYLEN]) {
  uint8 mac[POLY1305_MAC_SIZE];

  if (src_len < CHACHA20POLY1305_AUTHTAGLEN)
    return false;
  chacha20poly1305_decrypt_get_mac(dst, src, src_len - CHACHA20POLY1305_AUTHTAGLEN, ad, ad_len, nonce, key, mac);
  return memcmp_crypto(mac, src + src_len - CHACHA20POLY1305_AUTHTAGLEN, CHACHA20POLY1305_AUTHTAGLEN) == 0;
}

void xchacha20poly1305_encrypt(uint8 *dst, const uint8 *src, const size_t src_len,
			       const uint8 *ad, const size_t ad_len,
			       const uint8 nonce[XCHACHA20POLY1305_NONCELEN],
			       const uint8 key[CHACHA20POLY1305_KEYLEN])
{
  __aligned(16) uint8 derived_key[CHACHA20POLY1305_KEYLEN];

	hchacha20(derived_key, nonce, key);
	chacha20poly1305_encrypt(dst, src, src_len, ad, ad_len, ReadLE64(nonce + 16), derived_key);
	memzero_crypto(derived_key, CHACHA20POLY1305_KEYLEN);
}

bool xchacha20poly1305_decrypt(uint8 *dst, const uint8 *src, const size_t src_len,
			       const uint8 *ad, const size_t ad_len,
			       const uint8 nonce[XCHACHA20POLY1305_NONCELEN],
			       const uint8 key[CHACHA20POLY1305_KEYLEN]) {
  bool ret;
  __aligned(16) uint8 derived_key[CHACHA20POLY1305_KEYLEN];

	hchacha20(derived_key, nonce, key);
	ret = chacha20poly1305_decrypt(dst, src, src_len, ad, ad_len, ReadLE64(nonce + 16), derived_key);
	memzero_crypto(derived_key, CHACHA20POLY1305_KEYLEN);

	return ret;
}

void chacha20_streaming_init(chacha20_streaming *state, uint8 key[CHACHA20POLY1305_KEYLEN]) {
  state->left = 0;
  uint32 *st = state->state;
  WriteLE64((uint8*)st, 0x3320646e61707865);
  WriteLE64((uint8*)st + 8, 0x6b20657479622d32);
  Write64((uint8*)st + 16, Read64(key + 0));
  Write64((uint8*)st + 24, Read64(key + 8));
  Write64((uint8*)st + 32, Read64(key + 16));
  Write64((uint8*)st + 40, Read64(key + 24));
  Write64((uint8*)st + 48, 0);
  Write64((uint8*)st + 56, 0);
}

void chacha20_streaming_crypt(chacha20_streaming *state, uint8 *dst, size_t size) {
  uint32 left = state->left;
  while (size) {
    if (left == 0) {
      memset(state->buf, 0, sizeof(state->buf));
      chacha20_crypt((struct chacha20_ctx *)state->state, state->buf, state->buf, sizeof(state->buf));
      left = 64;
    }
    size_t step = left > size ? size : left;
    crypto_xor(postinc(dst, step), state->buf + 64 - exch(left, left - (uint32)step), exch(size, size - step));
  }
  state->left = left;
}


