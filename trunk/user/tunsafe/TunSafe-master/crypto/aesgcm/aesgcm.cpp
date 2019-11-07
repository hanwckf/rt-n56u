#include "stdafx.h"
#include "tunsafe_types.h"
#include "tunsafe_endian.h"
#include "tunsafe_cpu.h"
#include "crypto/aesgcm/aes.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
//#include <Windows.h>
#include "crypto/chacha20poly1305.h"
#define AESNIGCM_ASM 1
#define AESGCM_ASM 1
#define AESNI_GCM 1

// We only implement AES stuff on X86-64
#if WITH_AESGCM

extern "C" {
void gcm_init_clmul(aesgcm_u128 Htable[16],const uint64 Xi[2]);
void gcm_gmult_clmul(uint64 Xi[2],const aesgcm_u128 Htable[16]);
void gcm_ghash_clmul(uint64 Xi[2],const aesgcm_u128 Htable[16],const uint8 *inp,size_t len);
void gcm_init_avx(aesgcm_u128 Htable[16],const uint64 Xi[2]);
void gcm_gmult_avx(uint64 Xi[2],const aesgcm_u128 Htable[16]);
void gcm_ghash_avx(uint64 Xi[2],const aesgcm_u128 Htable[16],const uint8 *inp,size_t len);
void gcm_gmult_4bit(uint64 Xi[2], const aesgcm_u128 Htable[16]);
void gcm_ghash_4bit(uint64 Xi[2],const aesgcm_u128 Htable[16], const uint8 *inp,size_t len);

// ivec points to Yi followed by Xi
// h_and_htable points at h and htable from the static context
size_t aesni_gcm_encrypt(const uint8 *in,uint8 *out,size_t len,const void *key,uint8 ivec_and_xi[16],uint64 *h_and_htable);
size_t aesni_gcm_decrypt(const uint8 *in,uint8 *out,size_t len,const void *key,uint8 ivec_and_xi[16],uint64 *h_and_htable);
void aesni_ctr32_encrypt_blocks(const void *in, void *out, size_t blocks, const AesContext *key, const uint8 *ivec);
void aesni_encrypt(const void *inp, void *out, const AesContext *key);
void aesni_decrypt(const void *inp, void *out, const AesContext *key);
int aesni_set_encrypt_key(const unsigned char *inp, int bits, AesContext *key);
int aesni_set_decrypt_key(const unsigned char *inp, int bits, AesContext *key);
};


#define GCM_MUL(ctx,Xi)  (*gcm_gmult_p)(ctx->Xi.u,sctx->Htable)
#define GHASH(ctx,in,len) (*gcm_ghash_p)(ctx->Xi.u,sctx->Htable,in,len)
#define GHASH_CHUNK       (3*1024)

void CRYPTO_gcm128_aad(AesGcm128TempContext *ctx,const uint8 *aad,size_t len) {
  size_t i;
  unsigned int n;
  AesGcm128StaticContext *sctx = ctx->sctx;
  uint64 alen = ctx->len.u[0];
  void (*gcm_gmult_p)(uint64 Xi[2],const aesgcm_u128 Htable[16])  = sctx->gmult;
  void (*gcm_ghash_p)(uint64 Xi[2],const aesgcm_u128 Htable[16], const uint8 *inp,size_t len) = sctx->ghash;

  assert(!ctx->len.u[1]);
//  if () return -2;
  alen += len;
//  if (alen>(uint64(1)<<61) || (sizeof(len)==8 && alen<len))
//    return -1;
  ctx->len.u[0] = alen;

  n = ctx->ares;
  if (n) {
    while (n && len) {
      ctx->Xi.c[n] ^= *(aad++);
      --len;
      n = (n+1)%16;
    }
    if (n==0) GCM_MUL(ctx,Xi);
    else {
      ctx->ares = n;
      return;
    }
  }

#ifdef GHASH
  if ((i = (len&(size_t)-16))) {
    GHASH(ctx,aad,i);
    aad += i;
    len -= i;
  }
#else
  while (len>=16) {
    for (i=0; i<16; ++i) ctx->Xi.c[i] ^= aad[i];
    GCM_MUL(ctx,Xi);
    aad += 16;
    len -= 16;
  }
#endif
  if (len) {
    n = (unsigned int)len;
    for (i=0; i<len; ++i) ctx->Xi.c[i] ^= aad[i];
  }

  ctx->ares = n;
}

void CRYPTO_gcm128_encrypt_ctr32(AesGcm128TempContext *ctx, const uint8 *in, uint8 *out, size_t len) {
  unsigned int n, ctr;
  size_t i;
  AesGcm128StaticContext *sctx = ctx->sctx;
  uint64        mlen  = ctx->len.u[1];
  void (*gcm_gmult_p)(uint64 Xi[2],const aesgcm_u128 Htable[16])  = sctx->gmult;
  void (*gcm_ghash_p)(uint64 Xi[2],const aesgcm_u128 Htable[16], const uint8 *inp,size_t len) = sctx->ghash;
  mlen += len;
//  if (mlen>((uint64(1)<<36)-32) || (sizeof(len)==8 && mlen<len))
//    return -1;
  ctx->len.u[1] = mlen;

  if (ctx->ares) {
    /* First call to encrypt finalizes GHASH(AAD) */
    GCM_MUL(ctx,Xi);
    ctx->ares = 0;
  }
  n = ctx->mres;
  if (n) {
    while (n && len) {
      ctx->Xi.c[n] ^= *(out++) = *(in++)^ctx->EKi.c[n];
      --len;
      n = (n+1)%16;
    }
    if (n==0) GCM_MUL(ctx,Xi);
    else {
      ctx->mres = n;
      return;
    }
  }

#if defined(AESNI_GCM)
  if (sctx->use_aesni_gcm_crypt && len >= 0x120) {
    // |aesni_gcm_encrypt| may not process all the input given to it. It may
    // not process *any* of its input if it is deemed too small.
    size_t bulk = aesni_gcm_encrypt(in, out, len, &sctx->aes, ctx->Yi.c, sctx->H.u);
    in += bulk;
    out += bulk;
    len -= bulk;
  }
#endif
  ctr = ReadBE32(ctx->Yi.c + 12);

#if defined(STRICT_ALIGNMENT)
  if (((size_t)in | (size_t)out) % sizeof(size_t) != 0) {
    for (i = 0; i<len; ++i) {
      if (n == 0) {
        aesni_encrypt(ctx->Yi.c, ctx->EKi.c, &sctx->aes);
        ++ctr;
        WriteBE32(ctx->Yi.c + 12, ctr);
      }
      ctx->Xi.c[n] ^= out[i] = in[i] ^ ctx->EKi.c[n];
      n = (n + 1) % 16;
      if (n == 0)
        GCM_MUL(ctx, Xi);
    }
    ctx->mres = n;
    return;
  }
#endif
  while (len>=GHASH_CHUNK) {
    aesni_ctr32_encrypt_blocks(in, out, GHASH_CHUNK / 16, &sctx->aes, ctx->Yi.c);
    GHASH(ctx, out, GHASH_CHUNK);
    ctr += GHASH_CHUNK / 16;
    WriteBE32(ctx->Yi.c + 12, ctr);
    in += GHASH_CHUNK;
    out += GHASH_CHUNK;
    len -= GHASH_CHUNK;
  }
  if ((i = (len&(size_t)-16))) {
    aesni_ctr32_encrypt_blocks(in, out, i / 16, &sctx->aes, ctx->Yi.c);
    GHASH(ctx, out, i);
    ctr += (uint32)(i / 16);
    WriteBE32(ctx->Yi.c + 12, ctr);
    out += i;
    in += i;
    len -= i;
  }
  if (len) {
    aesni_encrypt(ctx->Yi.c, ctx->EKi.c, &sctx->aes);
    ++ctr;
    WriteBE32(ctx->Yi.c+12,ctr);
    while (len--) {
      ctx->Xi.c[n] ^= out[n] = in[n] ^ ctx->EKi.c[n];
      ++n;
    }
  }
  ctx->mres = n;
}

void CRYPTO_gcm128_decrypt_ctr32(AesGcm128TempContext *ctx, const uint8 *in, uint8 *out, size_t len) {
  unsigned int n, ctr;
  size_t i;
  uint64        mlen  = ctx->len.u[1];
  AesGcm128StaticContext *sctx = ctx->sctx;
  void (*gcm_gmult_p)(uint64 Xi[2],const aesgcm_u128 Htable[16])  = sctx->gmult;
  void (*gcm_ghash_p)(uint64 Xi[2],const aesgcm_u128 Htable[16],  const uint8 *inp,size_t len) = sctx->ghash;

  mlen += len;
//  if (mlen>((uint64(1)<<36)-32) || (sizeof(len)==8 && mlen<len))
//    return -1;
  ctx->len.u[1] = mlen;

  if (ctx->ares) {
    /* First call to decrypt finalizes GHASH(AAD) */
    GCM_MUL(ctx,Xi);
    ctx->ares = 0;
  }

  n = ctx->mres;
  if (n) {
    while (n && len) {
      uint8 c = *(in++);
      *(out++) = c^ctx->EKi.c[n];
      ctx->Xi.c[n] ^= c;
      --len;
      n = (n+1)%16;
    }
    if (n==0) GCM_MUL (ctx,Xi);
    else {
      ctx->mres = n;
      return;
    }
  }

#if defined(AESNI_GCM)
  if (sctx->use_aesni_gcm_crypt) {
    // |aesni_gcm_decrypt| may not process all the input given to it. It may
    // not process *any* of its input if it is deemed too small.
    size_t bulk = aesni_gcm_decrypt(in, out, len, &sctx->aes, ctx->Yi.c, sctx->H.u);
    in += bulk;
    out += bulk;
    len -= bulk;
  }
#endif
  ctr = ReadBE32(ctx->Yi.c + 12);

#if defined(STRICT_ALIGNMENT)
  if (((size_t)in|(size_t)out)%sizeof(size_t) != 0) {
    for (i=0;i<len;++i) {
      uint8 c;
      if (n==0) {
        aesni_encrypt(ctx->Yi.c, ctx->EKi.c, key);
        ++ctr;
        WriteBE32(ctx->Yi.c+12,ctr);
      }
      c = in[i];
      out[i] = c^ctx->EKi.c[n];
      ctx->Xi.c[n] ^= c;
      n = (n+1)%16;
      if (n==0)
        GCM_MUL(ctx,Xi);
    }
    ctx->mres = n;
    return;
  }
#endif
  while (len >= GHASH_CHUNK) {
    GHASH(ctx, in, GHASH_CHUNK);
    aesni_ctr32_encrypt_blocks(in, out, GHASH_CHUNK / 16, &sctx->aes, ctx->Yi.c);
    ctr += GHASH_CHUNK / 16;
    WriteBE32(ctx->Yi.c + 12, ctr);
    in += GHASH_CHUNK;
    out += GHASH_CHUNK;
    len -= GHASH_CHUNK;
  }
  if ((i = (len&(size_t)-16))) {
    GHASH(ctx, in, i);
    aesni_ctr32_encrypt_blocks(in, out, i / 16, &sctx->aes, ctx->Yi.c);
    ctr += (uint32)(i / 16);
    WriteBE32(ctx->Yi.c + 12, ctr);
    out += i;
    in += i;
    len -= i;
  }
  if (len) {
    aesni_encrypt(ctx->Yi.c, ctx->EKi.c, &sctx->aes);
    ++ctr;
    WriteBE32(ctx->Yi.c+12,ctr);
    while (len--) {
      uint8 c = in[n];
      ctx->Xi.c[n] ^= c;
      out[n] = c^ctx->EKi.c[n];
      ++n;
    }
  }
  ctx->mres = n;
}

void CRYPTO_gcm128_finish(AesGcm128TempContext *ctx,uint8 *tag, size_t len) {
  uint64 alen = ctx->len.u[0]<<3;
  uint64 clen = ctx->len.u[1]<<3;
  AesGcm128StaticContext *sctx = ctx->sctx;
  void (*gcm_gmult_p)(uint64 Xi[2],const aesgcm_u128 Htable[16])  = sctx->gmult;

  if (ctx->mres || ctx->ares)
    GCM_MUL(ctx,Xi);

  alen = ToBE64(alen);
  clen = ToBE64(clen);

  ctx->Xi.u[0] ^= alen;
  ctx->Xi.u[1] ^= clen;
  GCM_MUL(ctx,Xi);

  ctx->Xi.u[0] ^= ctx->EK0.u[0];
  ctx->Xi.u[1] ^= ctx->EK0.u[1];

  memcpy(tag, ctx->Xi.c,len);
}

#define REDUCE1BIT(V) do { \
  if (sizeof(size_t)==8) { \
    uint64 T = 0xe100000000000000ull & (0-(V.lo&1)); \
    V.lo  = (V.hi<<63)|(V.lo>>1); \
    V.hi  = (V.hi>>1 )^T; \
  } else { \
    uint32 T = 0xe1000000U & (0-(uint32)(V.lo&1)); \
    V.lo  = (V.hi<<63)|(V.lo>>1); \
    V.hi  = (V.hi>>1 )^((uint64)T<<32); \
  } \
} while(0)

static void gcm_init_4bit(aesgcm_u128 Htable[16], uint64 H[2]) {
  aesgcm_u128 V;

  Htable[0].hi = 0;
  Htable[0].lo = 0;
  V.hi = H[0];
  V.lo = H[1];

  Htable[8] = V;
  REDUCE1BIT(V);
  Htable[4] = V;
  REDUCE1BIT(V);
  Htable[2] = V;
  REDUCE1BIT(V);
  Htable[1] = V;
  Htable[3].hi  = V.hi^Htable[2].hi, Htable[3].lo  = V.lo^Htable[2].lo;
  V=Htable[4];
  Htable[5].hi  = V.hi^Htable[1].hi, Htable[5].lo  = V.lo^Htable[1].lo;
  Htable[6].hi  = V.hi^Htable[2].hi, Htable[6].lo  = V.lo^Htable[2].lo;
  Htable[7].hi  = V.hi^Htable[3].hi, Htable[7].lo  = V.lo^Htable[3].lo;
  V=Htable[8];
  Htable[9].hi  = V.hi^Htable[1].hi, Htable[9].lo  = V.lo^Htable[1].lo;
  Htable[10].hi = V.hi^Htable[2].hi, Htable[10].lo = V.lo^Htable[2].lo;
  Htable[11].hi = V.hi^Htable[3].hi, Htable[11].lo = V.lo^Htable[3].lo;
  Htable[12].hi = V.hi^Htable[4].hi, Htable[12].lo = V.lo^Htable[4].lo;
  Htable[13].hi = V.hi^Htable[5].hi, Htable[13].lo = V.lo^Htable[5].lo;
  Htable[14].hi = V.hi^Htable[6].hi, Htable[14].lo = V.lo^Htable[6].lo;
  Htable[15].hi = V.hi^Htable[7].hi, Htable[15].lo = V.lo^Htable[7].lo;
}


#if !AESGCM_ASM
#define PACK(s)   ((size_t)(s)<<(sizeof(size_t)*8-16))
static const size_t rem_4bit[16] = {
  PACK(0x0000), PACK(0x1C20), PACK(0x3840), PACK(0x2460),
  PACK(0x7080), PACK(0x6CA0), PACK(0x48C0), PACK(0x54E0),
  PACK(0xE100), PACK(0xFD20), PACK(0xD940), PACK(0xC560),
  PACK(0x9180), PACK(0x8DA0), PACK(0xA9C0), PACK(0xB5E0)};

void gcm_gmult_4bit(uint64 Xi[2], const aesgcm_u128 Htable[16]) {
  aesgcm_u128 Z;
  int cnt = 15;
  size_t rem, nlo, nhi;
  const union { long one; char little; } is_endian = {1};

  nlo  = ((const uint8 *)Xi)[15];
  nhi  = nlo>>4;
  nlo &= 0xf;

  Z.hi = Htable[nlo].hi;
  Z.lo = Htable[nlo].lo;

  while (1) {
    rem  = (size_t)Z.lo&0xf;
    Z.lo = (Z.hi<<60)|(Z.lo>>4);
    Z.hi = (Z.hi>>4);
    if (sizeof(size_t)==8)
      Z.hi ^= rem_4bit[rem];
    else
      Z.hi ^= (uint64)rem_4bit[rem]<<32;

    Z.hi ^= Htable[nhi].hi;
    Z.lo ^= Htable[nhi].lo;

    if (--cnt<0)    break;

    nlo  = ((const uint8 *)Xi)[cnt];
    nhi  = nlo>>4;
    nlo &= 0xf;

    rem  = (size_t)Z.lo&0xf;
    Z.lo = (Z.hi<<60)|(Z.lo>>4);
    Z.hi = (Z.hi>>4);
    if (sizeof(size_t)==8)
      Z.hi ^= rem_4bit[rem];
    else
      Z.hi ^= (uint64)rem_4bit[rem]<<32;

    Z.hi ^= Htable[nlo].hi;
    Z.lo ^= Htable[nlo].lo;
  }
  Xi[0] = ToBE64(Z.hi);
  Xi[1] = ToBE64(Z.lo);
}

void gcm_ghash_4bit(uint64 Xi[2],const aesgcm_u128 Htable[16], const uint8 *inp,size_t len) {
    aesgcm_u128 Z;
    int cnt;
    size_t rem, nlo, nhi;

    do {
      cnt  = 15;
      nlo  = ((const uint8 *)Xi)[15];
      nlo ^= inp[15];
      nhi  = nlo>>4;
      nlo &= 0xf;

      Z.hi = Htable[nlo].hi;
      Z.lo = Htable[nlo].lo;

      while (1) {
        rem  = (size_t)Z.lo&0xf;
        Z.lo = (Z.hi<<60)|(Z.lo>>4);
        Z.hi = (Z.hi>>4);
        if (sizeof(size_t)==8)
          Z.hi ^= rem_4bit[rem];
        else
          Z.hi ^= (uint64)rem_4bit[rem]<<32;

        Z.hi ^= Htable[nhi].hi;
        Z.lo ^= Htable[nhi].lo;

        if (--cnt<0)    break;

        nlo  = ((const uint8 *)Xi)[cnt];
        nlo ^= inp[cnt];
        nhi  = nlo>>4;
        nlo &= 0xf;

        rem  = (size_t)Z.lo&0xf;
        Z.lo = (Z.hi<<60)|(Z.lo>>4);
        Z.hi = (Z.hi>>4);
        if (sizeof(size_t)==8)
          Z.hi ^= rem_4bit[rem];
        else
          Z.hi ^= (uint64)rem_4bit[rem]<<32;

        Z.hi ^= Htable[nlo].hi;
        Z.lo ^= Htable[nlo].lo;
      }
    Xi[0] = ToBE64(Z.hi);
    Xi[1] = ToBE64(Z.lo);

    } while (inp+=16, len-=16);
}
#endif

void CRYPTO_gcm128_init(AesGcm128StaticContext *ctx, const uint8 *key, int key_size) {
  memset(ctx,0,sizeof(*ctx));
  ctx->use_aesni_gcm_crypt = X86_PCAP_MOVBE;
  aesni_set_encrypt_key(key, key_size, &ctx->aes);
  aesni_encrypt(ctx->H.c,ctx->H.c, &ctx->aes);
  ctx->H.u[0] = ToBE64(ctx->H.u[0]);
  ctx->H.u[1] = ToBE64(ctx->H.u[1]);
  if (X86_PCAP_AVX) {
    gcm_init_avx(ctx->Htable,ctx->H.u);
    ctx->gmult = gcm_gmult_avx;
    ctx->ghash = gcm_ghash_avx;
  } else if (X86_PCAP_PCLMULQDQ) {
    gcm_init_clmul(ctx->Htable,ctx->H.u);
    ctx->gmult = gcm_gmult_clmul;
    ctx->ghash = gcm_ghash_clmul;
  } else {
    gcm_init_4bit(ctx->Htable, ctx->H.u);
    ctx->gmult = gcm_gmult_4bit;
    ctx->ghash = gcm_ghash_4bit;
  }
}

void CRYPTO_gcm128_setiv(AesGcm128TempContext *ctx, AesGcm128StaticContext *sctx, const unsigned char *iv, size_t len) {
  unsigned int ctr;
  void (*gcm_gmult_p)(uint64 Xi[2],const aesgcm_u128 Htable[16])  = sctx->gmult;

  ctx->sctx = sctx;
  ctx->Yi.u[0]  = 0;
  ctx->Yi.u[1]  = 0;
  ctx->Xi.u[0]  = 0;
  ctx->Xi.u[1]  = 0;
  ctx->len.u[0] = 0;  /* AAD length */
  ctx->len.u[1] = 0;  /* message length */
  ctx->ares = 0;
  ctx->mres = 0;

  if (len==12) {
    memcpy(ctx->Yi.c,iv,12);
    ctx->Yi.c[15]=1;
    ctr=1;
  } else {
    size_t i;
    uint64 len0 = len;

    while (len>=16) {
      for (i=0; i<16; ++i) ctx->Yi.c[i] ^= iv[i];
      GCM_MUL(ctx,Yi);
      iv += 16;
      len -= 16;
    }
    if (len) {
      for (i=0; i<len; ++i) ctx->Yi.c[i] ^= iv[i];
      GCM_MUL(ctx,Yi);
    }
    len0 <<= 3;
    ctx->Yi.u[1]  ^= ToBE64(len0);

    GCM_MUL(ctx,Yi);

    ctr = ToBE32(ctx->Yi.d[3]);
  }

  aesni_encrypt(ctx->Yi.c, ctx->EK0.c, &sctx->aes);
  ++ctr;
  ctx->Yi.d[3] = ToBE32(ctr);
}

union AesGcmIV {
  uint32 nonce[3];
  uint8 nonceb[12];
};

void aesgcm_encrypt(uint8 *dst, const uint8 *src, const size_t src_len,
                    const uint8 *ad, const size_t ad_len,
                    const uint64 nonce, AesGcm128StaticContext *sctx) {
  AesGcm128TempContext ctx;
  AesGcmIV iv;

  WriteLE64(iv.nonce, nonce);
  iv.nonce[2] = 0;

  CRYPTO_gcm128_setiv(&ctx, sctx, iv.nonceb, sizeof(iv));
  CRYPTO_gcm128_aad(&ctx, ad, ad_len);
  CRYPTO_gcm128_encrypt_ctr32(&ctx, src, dst, src_len);
  CRYPTO_gcm128_finish(&ctx, dst + src_len, 16);
}

void aesgcm_decrypt_get_mac(uint8 *dst, const uint8 *src, const size_t src_len,
                            const uint8 *ad, const size_t ad_len,
                            const uint64 nonce, AesGcm128StaticContext *sctx,
                            uint8 mac[16]) {
  AesGcm128TempContext ctx;
  AesGcmIV iv;

  WriteLE64(iv.nonce, nonce);
  iv.nonce[2] = 0;

  CRYPTO_gcm128_setiv(&ctx, sctx, iv.nonceb, sizeof(iv));
  CRYPTO_gcm128_aad(&ctx, ad, ad_len);
  CRYPTO_gcm128_decrypt_ctr32(&ctx, src, dst, src_len);
  CRYPTO_gcm128_finish(&ctx, mac, 16);
}

#if 1

/*
* GCM test vectors from:
*
* http://csrc.nist.gov/groups/STM/cavp/documents/mac/gcmtestvectors.zip
*/
#define MAX_TESTS   6

static int key_index[MAX_TESTS] =
{ 0, 0, 1, 1, 1, 1 };

static uint8 key[MAX_TESTS][32] =
{
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
  0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
  0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
  0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08 },  
};

static size_t iv_len[MAX_TESTS] =
{ 12, 12, 12, 12, 8, 60 };

static int iv_index[MAX_TESTS] =
{ 0, 0, 1, 1, 1, 2 };

static uint8 iv[MAX_TESTS][64] =
{
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00 },
  { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
  0xde, 0xca, 0xf8, 0x88 },
  { 0x93, 0x13, 0x22, 0x5d, 0xf8, 0x84, 0x06, 0xe5,
  0x55, 0x90, 0x9c, 0x5a, 0xff, 0x52, 0x69, 0xaa, 
  0x6a, 0x7a, 0x95, 0x38, 0x53, 0x4f, 0x7d, 0xa1,
  0xe4, 0xc3, 0x03, 0xd2, 0xa3, 0x18, 0xa7, 0x28, 
  0xc3, 0xc0, 0xc9, 0x51, 0x56, 0x80, 0x95, 0x39,
  0xfc, 0xf0, 0xe2, 0x42, 0x9a, 0x6b, 0x52, 0x54, 
  0x16, 0xae, 0xdb, 0xf5, 0xa0, 0xde, 0x6a, 0x57,
  0xa6, 0x37, 0xb3, 0x9b }, 
};

static size_t add_len[MAX_TESTS] =
{ 0, 0, 0, 20, 20, 20 };

int add_index[MAX_TESTS] =
{ 0, 0, 0, 1, 1, 1 };

static uint8 additional[MAX_TESTS][64] =
{
  { 0x00 },
  { 0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
  0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 
  0xab, 0xad, 0xda, 0xd2 },
};

static size_t pt_len[MAX_TESTS] =
{ 0, 16, 64, 60, 60, 60 };

static int pt_index[MAX_TESTS] =
{ 0, 0, 1, 1, 1, 1 };

static uint8 pt[MAX_TESTS][64] =
{
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
  0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
  0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
  0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
  0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
  0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
  0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
  0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55 },
};

static uint8 ct[MAX_TESTS * 3][64] =
{
  { 0x00 },
  { 0x03, 0x88, 0xda, 0xce, 0x60, 0xb6, 0xa3, 0x92,
  0xf3, 0x28, 0xc2, 0xb9, 0x71, 0xb2, 0xfe, 0x78 },
  { 0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24,
  0x4b, 0x72, 0x21, 0xb7, 0x84, 0xd0, 0xd4, 0x9c, 
  0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0,
  0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e, 
  0x21, 0xd5, 0x14, 0xb2, 0x54, 0x66, 0x93, 0x1c,
  0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05, 
  0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97,
  0x3d, 0x58, 0xe0, 0x91, 0x47, 0x3f, 0x59, 0x85 },
  { 0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24,
  0x4b, 0x72, 0x21, 0xb7, 0x84, 0xd0, 0xd4, 0x9c, 
  0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0,
  0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e, 
  0x21, 0xd5, 0x14, 0xb2, 0x54, 0x66, 0x93, 0x1c,
  0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05, 
  0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97,
  0x3d, 0x58, 0xe0, 0x91 },
  { 0x61, 0x35, 0x3b, 0x4c, 0x28, 0x06, 0x93, 0x4a,
  0x77, 0x7f, 0xf5, 0x1f, 0xa2, 0x2a, 0x47, 0x55, 
  0x69, 0x9b, 0x2a, 0x71, 0x4f, 0xcd, 0xc6, 0xf8,
  0x37, 0x66, 0xe5, 0xf9, 0x7b, 0x6c, 0x74, 0x23, 
  0x73, 0x80, 0x69, 0x00, 0xe4, 0x9f, 0x24, 0xb2,
  0x2b, 0x09, 0x75, 0x44, 0xd4, 0x89, 0x6b, 0x42, 
  0x49, 0x89, 0xb5, 0xe1, 0xeb, 0xac, 0x0f, 0x07,
  0xc2, 0x3f, 0x45, 0x98 },
  { 0x8c, 0xe2, 0x49, 0x98, 0x62, 0x56, 0x15, 0xb6,
  0x03, 0xa0, 0x33, 0xac, 0xa1, 0x3f, 0xb8, 0x94, 
  0xbe, 0x91, 0x12, 0xa5, 0xc3, 0xa2, 0x11, 0xa8,
  0xba, 0x26, 0x2a, 0x3c, 0xca, 0x7e, 0x2c, 0xa7, 
  0x01, 0xe4, 0xa9, 0xa4, 0xfb, 0xa4, 0x3c, 0x90,
  0xcc, 0xdc, 0xb2, 0x81, 0xd4, 0x8c, 0x7c, 0x6f, 
  0xd6, 0x28, 0x75, 0xd2, 0xac, 0xa4, 0x17, 0x03,
  0x4c, 0x34, 0xae, 0xe5 },
  { 0x00 },
  { 0x98, 0xe7, 0x24, 0x7c, 0x07, 0xf0, 0xfe, 0x41,
  0x1c, 0x26, 0x7e, 0x43, 0x84, 0xb0, 0xf6, 0x00 }, 
  { 0x39, 0x80, 0xca, 0x0b, 0x3c, 0x00, 0xe8, 0x41,
  0xeb, 0x06, 0xfa, 0xc4, 0x87, 0x2a, 0x27, 0x57, 
  0x85, 0x9e, 0x1c, 0xea, 0xa6, 0xef, 0xd9, 0x84,
  0x62, 0x85, 0x93, 0xb4, 0x0c, 0xa1, 0xe1, 0x9c, 
  0x7d, 0x77, 0x3d, 0x00, 0xc1, 0x44, 0xc5, 0x25,
  0xac, 0x61, 0x9d, 0x18, 0xc8, 0x4a, 0x3f, 0x47, 
  0x18, 0xe2, 0x44, 0x8b, 0x2f, 0xe3, 0x24, 0xd9,
  0xcc, 0xda, 0x27, 0x10, 0xac, 0xad, 0xe2, 0x56 },
  { 0x39, 0x80, 0xca, 0x0b, 0x3c, 0x00, 0xe8, 0x41,
  0xeb, 0x06, 0xfa, 0xc4, 0x87, 0x2a, 0x27, 0x57, 
  0x85, 0x9e, 0x1c, 0xea, 0xa6, 0xef, 0xd9, 0x84,
  0x62, 0x85, 0x93, 0xb4, 0x0c, 0xa1, 0xe1, 0x9c, 
  0x7d, 0x77, 0x3d, 0x00, 0xc1, 0x44, 0xc5, 0x25, 
  0xac, 0x61, 0x9d, 0x18, 0xc8, 0x4a, 0x3f, 0x47, 
  0x18, 0xe2, 0x44, 0x8b, 0x2f, 0xe3, 0x24, 0xd9,
  0xcc, 0xda, 0x27, 0x10 }, 
  { 0x0f, 0x10, 0xf5, 0x99, 0xae, 0x14, 0xa1, 0x54,
  0xed, 0x24, 0xb3, 0x6e, 0x25, 0x32, 0x4d, 0xb8, 
  0xc5, 0x66, 0x63, 0x2e, 0xf2, 0xbb, 0xb3, 0x4f,
  0x83, 0x47, 0x28, 0x0f, 0xc4, 0x50, 0x70, 0x57, 
  0xfd, 0xdc, 0x29, 0xdf, 0x9a, 0x47, 0x1f, 0x75,
  0xc6, 0x65, 0x41, 0xd4, 0xd4, 0xda, 0xd1, 0xc9, 
  0xe9, 0x3a, 0x19, 0xa5, 0x8e, 0x8b, 0x47, 0x3f,
  0xa0, 0xf0, 0x62, 0xf7 }, 
  { 0xd2, 0x7e, 0x88, 0x68, 0x1c, 0xe3, 0x24, 0x3c,
  0x48, 0x30, 0x16, 0x5a, 0x8f, 0xdc, 0xf9, 0xff, 
  0x1d, 0xe9, 0xa1, 0xd8, 0xe6, 0xb4, 0x47, 0xef,
  0x6e, 0xf7, 0xb7, 0x98, 0x28, 0x66, 0x6e, 0x45, 
  0x81, 0xe7, 0x90, 0x12, 0xaf, 0x34, 0xdd, 0xd9,
  0xe2, 0xf0, 0x37, 0x58, 0x9b, 0x29, 0x2d, 0xb3, 
  0xe6, 0x7c, 0x03, 0x67, 0x45, 0xfa, 0x22, 0xe7,
  0xe9, 0xb7, 0x37, 0x3b }, 
  { 0x00 },
  { 0xce, 0xa7, 0x40, 0x3d, 0x4d, 0x60, 0x6b, 0x6e, 
  0x07, 0x4e, 0xc5, 0xd3, 0xba, 0xf3, 0x9d, 0x18 }, 
  { 0x52, 0x2d, 0xc1, 0xf0, 0x99, 0x56, 0x7d, 0x07, 
  0xf4, 0x7f, 0x37, 0xa3, 0x2a, 0x84, 0x42, 0x7d, 
  0x64, 0x3a, 0x8c, 0xdc, 0xbf, 0xe5, 0xc0, 0xc9, 
  0x75, 0x98, 0xa2, 0xbd, 0x25, 0x55, 0xd1, 0xaa, 
  0x8c, 0xb0, 0x8e, 0x48, 0x59, 0x0d, 0xbb, 0x3d, 
  0xa7, 0xb0, 0x8b, 0x10, 0x56, 0x82, 0x88, 0x38, 
  0xc5, 0xf6, 0x1e, 0x63, 0x93, 0xba, 0x7a, 0x0a, 
  0xbc, 0xc9, 0xf6, 0x62, 0x89, 0x80, 0x15, 0xad }, 
  { 0x52, 0x2d, 0xc1, 0xf0, 0x99, 0x56, 0x7d, 0x07, 
  0xf4, 0x7f, 0x37, 0xa3, 0x2a, 0x84, 0x42, 0x7d,  
  0x64, 0x3a, 0x8c, 0xdc, 0xbf, 0xe5, 0xc0, 0xc9, 
  0x75, 0x98, 0xa2, 0xbd, 0x25, 0x55, 0xd1, 0xaa, 
  0x8c, 0xb0, 0x8e, 0x48, 0x59, 0x0d, 0xbb, 0x3d, 
  0xa7, 0xb0, 0x8b, 0x10, 0x56, 0x82, 0x88, 0x38, 
  0xc5, 0xf6, 0x1e, 0x63, 0x93, 0xba, 0x7a, 0x0a, 
  0xbc, 0xc9, 0xf6, 0x62 }, 
  { 0xc3, 0x76, 0x2d, 0xf1, 0xca, 0x78, 0x7d, 0x32,
  0xae, 0x47, 0xc1, 0x3b, 0xf1, 0x98, 0x44, 0xcb, 
  0xaf, 0x1a, 0xe1, 0x4d, 0x0b, 0x97, 0x6a, 0xfa,
  0xc5, 0x2f, 0xf7, 0xd7, 0x9b, 0xba, 0x9d, 0xe0, 
  0xfe, 0xb5, 0x82, 0xd3, 0x39, 0x34, 0xa4, 0xf0,
  0x95, 0x4c, 0xc2, 0x36, 0x3b, 0xc7, 0x3f, 0x78, 
  0x62, 0xac, 0x43, 0x0e, 0x64, 0xab, 0xe4, 0x99,
  0xf4, 0x7c, 0x9b, 0x1f }, 
  { 0x5a, 0x8d, 0xef, 0x2f, 0x0c, 0x9e, 0x53, 0xf1,
  0xf7, 0x5d, 0x78, 0x53, 0x65, 0x9e, 0x2a, 0x20, 
  0xee, 0xb2, 0xb2, 0x2a, 0xaf, 0xde, 0x64, 0x19,
  0xa0, 0x58, 0xab, 0x4f, 0x6f, 0x74, 0x6b, 0xf4, 
  0x0f, 0xc0, 0xc3, 0xb7, 0x80, 0xf2, 0x44, 0x45,
  0x2d, 0xa3, 0xeb, 0xf1, 0xc5, 0xd8, 0x2c, 0xde, 
  0xa2, 0x41, 0x89, 0x97, 0x20, 0x0e, 0xf8, 0x2e,
  0x44, 0xae, 0x7e, 0x3f }, 
};

static uint8 tag[MAX_TESTS * 3][16] =
{
  { 0x58, 0xe2, 0xfc, 0xce, 0xfa, 0x7e, 0x30, 0x61,
  0x36, 0x7f, 0x1d, 0x57, 0xa4, 0xe7, 0x45, 0x5a },
  { 0xab, 0x6e, 0x47, 0xd4, 0x2c, 0xec, 0x13, 0xbd,
  0xf5, 0x3a, 0x67, 0xb2, 0x12, 0x57, 0xbd, 0xdf },
  { 0x4d, 0x5c, 0x2a, 0xf3, 0x27, 0xcd, 0x64, 0xa6,
  0x2c, 0xf3, 0x5a, 0xbd, 0x2b, 0xa6, 0xfa, 0xb4 }, 
  { 0x5b, 0xc9, 0x4f, 0xbc, 0x32, 0x21, 0xa5, 0xdb,
  0x94, 0xfa, 0xe9, 0x5a, 0xe7, 0x12, 0x1a, 0x47 },
  { 0x36, 0x12, 0xd2, 0xe7, 0x9e, 0x3b, 0x07, 0x85,
  0x56, 0x1b, 0xe1, 0x4a, 0xac, 0xa2, 0xfc, 0xcb },
  { 0x61, 0x9c, 0xc5, 0xae, 0xff, 0xfe, 0x0b, 0xfa,
  0x46, 0x2a, 0xf4, 0x3c, 0x16, 0x99, 0xd0, 0x50 },
  { 0xcd, 0x33, 0xb2, 0x8a, 0xc7, 0x73, 0xf7, 0x4b,
  0xa0, 0x0e, 0xd1, 0xf3, 0x12, 0x57, 0x24, 0x35 },
  { 0x2f, 0xf5, 0x8d, 0x80, 0x03, 0x39, 0x27, 0xab,
  0x8e, 0xf4, 0xd4, 0x58, 0x75, 0x14, 0xf0, 0xfb }, 
  { 0x99, 0x24, 0xa7, 0xc8, 0x58, 0x73, 0x36, 0xbf,
  0xb1, 0x18, 0x02, 0x4d, 0xb8, 0x67, 0x4a, 0x14 },
  { 0x25, 0x19, 0x49, 0x8e, 0x80, 0xf1, 0x47, 0x8f,
  0x37, 0xba, 0x55, 0xbd, 0x6d, 0x27, 0x61, 0x8c }, 
  { 0x65, 0xdc, 0xc5, 0x7f, 0xcf, 0x62, 0x3a, 0x24,
  0x09, 0x4f, 0xcc, 0xa4, 0x0d, 0x35, 0x33, 0xf8 }, 
  { 0xdc, 0xf5, 0x66, 0xff, 0x29, 0x1c, 0x25, 0xbb,
  0xb8, 0x56, 0x8f, 0xc3, 0xd3, 0x76, 0xa6, 0xd9 }, 
  { 0x53, 0x0f, 0x8a, 0xfb, 0xc7, 0x45, 0x36, 0xb9,
  0xa9, 0x63, 0xb4, 0xf1, 0xc4, 0xcb, 0x73, 0x8b }, 
  { 0xd0, 0xd1, 0xc8, 0xa7, 0x99, 0x99, 0x6b, 0xf0,
  0x26, 0x5b, 0x98, 0xb5, 0xd4, 0x8a, 0xb9, 0x19 }, 
  { 0xb0, 0x94, 0xda, 0xc5, 0xd9, 0x34, 0x71, 0xbd,
  0xec, 0x1a, 0x50, 0x22, 0x70, 0xe3, 0xcc, 0x6c }, 
  { 0x76, 0xfc, 0x6e, 0xce, 0x0f, 0x4e, 0x17, 0x68,
  0xcd, 0xdf, 0x88, 0x53, 0xbb, 0x2d, 0x55, 0x1b }, 
  { 0x3a, 0x33, 0x7d, 0xbf, 0x46, 0xa7, 0x92, 0xc4,
  0x5e, 0x45, 0x49, 0x13, 0xfe, 0x2e, 0xa8, 0xf2 }, 
  { 0xa4, 0x4a, 0x82, 0x66, 0xee, 0x1c, 0x8e, 0xb0,
  0xc8, 0xb5, 0xd4, 0xcf, 0x5a, 0xe9, 0xf1, 0x9a }, 
};

int gcm_self_test()
{
  uint8 buf[64];
  uint8 tag_buf[16];
  int i, j;

  AesGcm128TempContext ctx;
  AesGcm128StaticContext sctx;


  {
    AesContext aes;
    uint8  key[16] = {43,126,21,22,40,174,210,166,171,247,21,136,9,207,79,60};
    uint8   in[16] = {107,193,190,226,46,64,159,150,233,61,126,17,115,147,23,42};
    uint8   out[16] = {58,215,123,180,13,122,54,96,168,158,202,243,36,102,239,151}, t[16];
    aesni_set_encrypt_key(key, 128, &aes);
    aesni_encrypt(in, t, &aes);
    if (memcmp(t, out,16)) { printf("AES test fail!\n"); return 1; }
    aesni_set_decrypt_key(key, 128, &aes);
    aesni_decrypt(out, t, &aes);
    if (memcmp(t, in,16)) { printf("AES test fail!\n"); return 1; }
  }

  uint8 correct[] = { 62,85,184,249,224,220,4,77,201,216,202,172,121,7,25,200, };
  if (0) {
    uint8 buf[512 + 16];
    for (size_t i = 0; i < 512; i++)
      buf[i] = (uint8)(i >> 4);// 0x11;
    uint8 buf2[512 + 16];
    for (size_t i = 0; i < 512; i++)
      buf2[i] = buf[i];

    size_t pp = 0x60;

    CRYPTO_gcm128_init(&sctx, key[0], 128);
    
    sctx.use_aesni_gcm_crypt = 1;

    aesgcm_decrypt_get_mac(buf, buf, pp, NULL, 0, 1, &sctx, buf + pp);
    sctx.use_aesni_gcm_crypt = 0;
    aesgcm_decrypt_get_mac(buf2, buf2, pp, NULL, 0, 1, &sctx, buf2 + pp);
    //aesgcm_encrypt(buf, buf, 0x120 + 32, NULL, 0, 1, &sctx);

    for (size_t i = 0; i < 16; i++)
      printf("%d,", buf[pp + i]);
    printf("\n");
    for (size_t i = 0; i < 16; i++)
      printf("%d,", buf2[pp + i]);
    printf("\n");

    if (memcmp(buf2 + pp, buf + pp, 16) == 0)
      printf("CORRECT!!\n");
    else
      printf("******** FAIL ************\n");
//    for(size_t i = 0; i < 16; i++)
//      printf("%d,", buf[pp +i]);
    printf("\n");
  }
  return 0;

  for( j = 0; j < 3; j++ ) {
    int key_len = 128 + 64 * j;
    for( i = 0; i < MAX_TESTS; i++ ) {
      CRYPTO_gcm128_init(&sctx, key[key_index[i]], key_len);
      CRYPTO_gcm128_setiv(&ctx, &sctx, iv[iv_index[i]], iv_len[i]);
      CRYPTO_gcm128_aad(&ctx, additional[add_index[i]], add_len[i]);
      CRYPTO_gcm128_encrypt_ctr32(&ctx, pt[pt_index[i]], buf, pt_len[i]);
      CRYPTO_gcm128_finish(&ctx, tag_buf, 16);
      if(memcmp( buf, ct[j * 6 + i], pt_len[i] ) != 0 ||
         memcmp( tag_buf, tag[j * 6 + i], 16 ) != 0 ) {
        printf( "AES-GCM-%3d #%d (%s):  failed\n", key_len, i, "enc"  );
        return( 1 );
      }

      CRYPTO_gcm128_init(&sctx, key[key_index[i]], key_len);
      CRYPTO_gcm128_setiv(&ctx, &sctx, iv[iv_index[i]], iv_len[i]);
      CRYPTO_gcm128_aad(&ctx, additional[add_index[i]], add_len[i]);
      CRYPTO_gcm128_decrypt_ctr32(&ctx, ct[j * 6 + i], buf, pt_len[i]);
      CRYPTO_gcm128_finish(&ctx, tag_buf, 16);
      if(memcmp( buf, pt[pt_index[i]], pt_len[i] ) != 0 ||
         memcmp( tag_buf, tag[j * 6 + i], 16 ) != 0 ) {
        printf( "AES-GCM-%3d #%d (%s): failed\n", key_len, i, "dec"  );
        return( 1 );
      }
    }
  }

  return( 0 );
}

//int main() {
//  gcm_self_test();
//}
#endif

#endif  // #if WITH_AESGCM
