/*
    Wrapper around moxiebox'es implementation of SHA256 digest that
    mimics the API of the OpenSSL implementation.
*/

#ifndef __SHA_H_
#define __SHA_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include "../runtime/sandboxrt_crypto.h"
#define SHA256_DIGEST_LENGTH SHA256_BLOCK_SIZE
void SHA256_Init(SHA256_CTX *ctx);
void SHA256_Update(SHA256_CTX *ctx, const void *data, size_t len);
void SHA256_Final(unsigned char *md, SHA256_CTX *ctx);

#ifdef  __cplusplus
}
#endif

#endif
