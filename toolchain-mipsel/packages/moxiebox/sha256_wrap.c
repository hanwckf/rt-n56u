/*
    Wrapper around moxiebox'es implementation of SHA256 digest that
    mimics the API of the OpenSSL implementation.
*/

#include "sha.h"
#include "../runtime/sha256.c"

void
SHA256_Init(SHA256_CTX *ctx)
{
    sha256_init(ctx);
}

void
SHA256_Update(SHA256_CTX *ctx, const void *data, size_t len)
{
    sha256_update(ctx, data, len);
}

void
SHA256_Final(unsigned char *md, SHA256_CTX *ctx)
{
    sha256_final(ctx, md);
}
