/*
 *  sha1.h
 *
 *  Copyright (C) 1998, 2009
 *  Paul E. Jones <paulej@packetizer.com>
 *  All Rights Reserved
 *
 *****************************************************************************
 *  $Id: sha1.h 12 2009-06-22 19:34:25Z paulej $
 *****************************************************************************
 *
 *  Description:
 *      This class implements the Secure Hashing Standard as defined
 *      in FIPS PUB 180-1 published April 17, 1995.
 *
 *      Many of the variable names in the SHA1Context, especially the
 *      single character names, were used because those were the names
 *      used in the publication.
 *
 *      Please read the file sha1.c for more information.
 *
 */

#ifndef _SHA1_H_
#define _SHA1_H_

#include "tunsafe_types.h"

struct SHA1Context {
    uint32 state[5];
    uint64 length;
    uint8 buffer[64];
    uint32 pos;
};

void SHA1Reset(SHA1Context *ctx);
void SHA1Input(SHA1Context *ctx, const uint8 *input, size_t input_len);
void SHA1Finish(SHA1Context *ctx, uint8 digest[20]);
void SHA1Hash(const uint8 *data, int data_size, uint8 digest[20]);

struct SHA1HmacContext {
  SHA1Context sha1, sha2;
};

void SHA1HmacReset(SHA1HmacContext *hmac, const unsigned char *key, unsigned key_size);
void SHA1HmacInput(SHA1HmacContext *hmac, const unsigned char *input, unsigned input_size);
void SHA1HmacFinish(SHA1HmacContext *hmac, byte digest[20]);

#endif
