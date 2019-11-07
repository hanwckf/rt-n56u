// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
// Copyright (C) 1998, 2009, Paul E. Jones <paulej@packetizer.com>

#include "stdafx.h"
#include "crypto/sha/sha1.h"
#include <string.h>
#include "tunsafe_endian.h"

#define SHA1Rotate(word, bits) ((((word) << (bits)) & 0xFFFFFFFF) | ((word) >> (32-(bits))))

static void SHA1ProcessMessageBlock(SHA1Context *ctx) {
  uint32    t, temp, W[80];

  for (t = 0; t < 16; t++)
    W[t] = ReadBE32(&ctx->buffer[t * 4]);

  for (t = 16; t < 80; t++)
    W[t] = SHA1Rotate( W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);

  uint32 A = ctx->state[0], B = ctx->state[1], C = ctx->state[2], D = ctx->state[3], E = ctx->state[4];

#define SHA1_ROUND(x) temp = SHA1Rotate(A, 5) + (x), E = D, D = C, C = SHA1Rotate(B, 30), B = A, A = temp
  for (t = 0; t < 20; t++)
    SHA1_ROUND(((B & C) | ((~B) & D)) + E + W[t] + 0x5A827999);
  for (t = 20; t < 40; t++)
    SHA1_ROUND((B ^ C ^ D) + E + W[t] + 0x6ED9EBA1);
  for (t = 40; t < 60; t++)
    SHA1_ROUND(((B & C) | (B & D) | (C & D)) + E + W[t] + 0x8F1BBCDC);
  for (t = 60; t < 80; t++)
    SHA1_ROUND((B ^ C ^ D) + E + W[t] + 0xCA62C1D6);
#undef SHA1_ROUND

  ctx->state[0] += A;
  ctx->state[1] += B;
  ctx->state[2] += C;
  ctx->state[3] += D;
  ctx->state[4] += E;
  ctx->pos = 0;
}

void SHA1Reset(SHA1Context *ctx) {
  ctx->length = ctx->pos = 0;
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xEFCDAB89;
  ctx->state[2] = 0x98BADCFE;
  ctx->state[3] = 0x10325476;
  ctx->state[4] = 0xC3D2E1F0;
}

void SHA1Finish(SHA1Context *ctx, uint8 digest[20]) {
  ctx->buffer[ctx->pos++] = 0x80;
  while (ctx->pos != 56) {
    if (ctx->pos == 64)
      SHA1ProcessMessageBlock(ctx);
    ctx->buffer[ctx->pos++] = 0;
  }
  WriteBE64(&ctx->buffer[56], ctx->length);
  SHA1ProcessMessageBlock(ctx);
  for (int i = 0; i < 5; i++)
    WriteBE32(digest + i * 4, ctx->state[i]);
}

void SHA1Input(SHA1Context *ctx, const uint8 *input, size_t input_len) {
  ctx->length += input_len * 8;
  while (input_len--) {
    ctx->buffer[ctx->pos++] = *input++;
    if (ctx->pos == 64)
      SHA1ProcessMessageBlock(ctx);
  }
}

void SHA1Hash(const uint8 *data, int data_size, uint8 digest[20]) {
  SHA1Context ctx;
  SHA1Reset(&ctx);
  SHA1Input(&ctx, data, data_size);
  SHA1Finish(&ctx, digest);
}

void SHA1HmacReset(SHA1HmacContext *hmac, const unsigned char *key, unsigned key_size) {
  byte temp[64];
  byte temp2[64];
  byte digest[20];
  int i;

  if (key_size > 64) {
    SHA1Hash(key, key_size, digest);
    key = digest;
    key_size = sizeof(digest);
  }

  for (i = 0; i != key_size; i++) {
    temp[i] = key[i] ^ 0x36;
    temp2[i] = key[i] ^ 0x5C;
  }
  for (; i != 64; i++) {
    temp[i] = 0x36;
    temp2[i] = 0x5C;
  }

  SHA1Reset(&hmac->sha1);
  SHA1Reset(&hmac->sha2);

  SHA1Input(&hmac->sha1, temp, sizeof(temp));
  SHA1Input(&hmac->sha2, temp2, sizeof(temp2));
}

void SHA1HmacInput(SHA1HmacContext *hmac, const unsigned char *input, unsigned input_size) {
  SHA1Input(&hmac->sha1, input, input_size);
}

void SHA1HmacFinish(SHA1HmacContext *hmac, byte digest[20]) {
  SHA1Finish(&hmac->sha1, digest);
  SHA1Input(&hmac->sha2, digest, 20);
  SHA1Finish(&hmac->sha2, digest);
}
