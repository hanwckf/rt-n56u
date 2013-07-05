/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2004 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Copyright (C) 2004  Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) 2000, 2001  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "common.h"
#include "dhcp6.h"
#include "dhcp6c_auth.h"

#define HMACMD5_KEYLENGTH	64
#define MD5_DIGESTLENGTH	16

typedef struct {
	md5_ctx_t md5ctx;
	unsigned char key[HMACMD5_KEYLENGTH];
} hmacmd5_t;


static void hmacmd5_init(hmacmd5_t *, const unsigned char *, unsigned int);
static void hmacmd5_sign(hmacmd5_t *, unsigned char *);
static int hmacmd5_verify(hmacmd5_t *, unsigned char *);


int dhcp6_validate_key(struct keyinfo *key)
{
	time_t now;

	if (key->expire == 0)	/* never expire */
		return 0;

	time(&now);
	if (now > key->expire)
		return -1;

	return 0;
}

int dhcp6_calc_mac(char *buf, size_t len, int proto UNUSED_PARAM,
		int alg, size_t off, struct keyinfo *key)
{
	hmacmd5_t ctx;
	unsigned char digest[MD5_DIGESTLENGTH];

	/* right now, we don't care about the protocol */

	if (alg != DHCP6_AUTHALG_HMACMD5)
		return -1;

	if (off + MD5_DIGESTLENGTH > len) {
		/*
		 * this should be assured by the caller, but check it here
		 * for safety.
		 */
		return -2;
	}

	hmacmd5_init(&ctx, key->secret, key->secretlen);
	md5_hash(&ctx.md5ctx, buf, len);
	hmacmd5_sign(&ctx, digest);

	memcpy(buf + off, digest, MD5_DIGESTLENGTH);

	return 0;
}

int dhcp6_verify_mac(char *buf, ssize_t len, int proto UNUSED_PARAM,
		 int alg, size_t off, struct keyinfo *key)
{
	hmacmd5_t ctx;
	unsigned char digest[MD5_DIGESTLENGTH];
	int result;

	/* right now, we don't care about the protocol */

	if (alg != DHCP6_AUTHALG_HMACMD5)
		return -1;

	if (off + MD5_DIGESTLENGTH > len)
		return -1;

	/*
	 * Copy the MAC value and clear the field.
	 * XXX: should we make a local working copy?
	 */
	memcpy(digest, buf + off, sizeof(digest));
	memset(buf + off, 0, sizeof(digest));

	hmacmd5_init(&ctx, key->secret, key->secretlen);
	md5_hash(&ctx.md5ctx, buf, len);
	result = hmacmd5_verify(&ctx, digest);

	/* copy back the digest value (XXX) */
	memcpy(buf + off, digest, sizeof(digest));

	return result;
}

int dhcp6_auth_replaycheck(int method, uint64_t prev, uint64_t current)
{

	if (method != DHCP6_AUTHRDM_MONOCOUNTER) {
		bb_error_msg("unsupported replay detection method (%d)", method);
		return -1;
	}

	log1("previous: %llx, current: %llx", SWAP_BE64(prev), SWAP_BE64(current));

	prev = SWAP_BE64(prev);
	current = SWAP_BE64(current);

	/*
	 * we call the singular point guilty, since we cannot guess
	 * whether the serial number is increasing or not.
	 */
	if (prev == (current ^ 0x8000000000000000ULL)) {
		bb_info_msg("detected a singular point");
		return 1;
	}

	return (((int64_t )(current - prev) > 0) ? 0 : 1);
}


/*
 * This code implements the HMAC-MD5 keyed hash algorithm
 * described in RFC 2104.
 */

#define PADLEN 64
#define IPAD   0x36
#define OPAD   0x5C

/*
 * Start HMAC-MD5 process.  Initialize an md5 context and digest the key.
 */
static void hmacmd5_init(hmacmd5_t *ctx,
				 const unsigned char *key, unsigned int len)
{
	unsigned char ipad[PADLEN];
	int i;

	memset(ctx->key, 0, sizeof(ctx->key));
	if (len > sizeof(ctx->key)) {
		md5_ctx_t md5ctx;

		md5_begin(&md5ctx);
		md5_hash(&md5ctx, key, len);
		md5_end(&md5ctx, ctx->key);
	} else {
		memcpy(ctx->key, key, len);
	}

	md5_begin(&ctx->md5ctx);
	memset(ipad, IPAD, sizeof(ipad));
	for (i = 0; i < PADLEN; i++) {
		ipad[i] ^= ctx->key[i];
	}

	md5_hash(&ctx->md5ctx, ipad, sizeof(ipad));
}

/*
 * Compute signature - finalize MD5 operation and reapply MD5.
 */
static void hmacmd5_sign(hmacmd5_t *ctx, unsigned char *digest)
{
	unsigned char opad[PADLEN];
	int i;

	md5_end(&ctx->md5ctx, digest);

	memset(opad, OPAD, sizeof(opad));
	for (i = 0; i < PADLEN; i++) {
		opad[i] ^= ctx->key[i];
	}

	md5_begin(&ctx->md5ctx);
	md5_hash(&ctx->md5ctx, opad, sizeof(opad));
	md5_hash(&ctx->md5ctx, digest, MD5_DIGESTLENGTH);
	md5_end(&ctx->md5ctx, digest);
	memset(ctx, 0, sizeof(*ctx));
}

/*
 * Verify signature - finalize MD5 operation and reapply MD5, then
 * compare to the supplied digest.
 */
static int hmacmd5_verify(hmacmd5_t *ctx, unsigned char *digest) {
	unsigned char newdigest[MD5_DIGESTLENGTH];

	hmacmd5_sign(ctx, newdigest);
	return memcmp(digest, newdigest, MD5_DIGESTLENGTH);
}
