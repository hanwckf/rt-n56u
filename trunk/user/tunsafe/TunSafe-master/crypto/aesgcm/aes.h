/**
 * Downloaded from
 *
 *  http://www.esat.kuleuven.ac.be/~rijmen/rijndael/rijndael-fst-3.0.zip
 *
 * rijndael-alg-fst.h
 *
 * @version 3.0 (December 2000)
 *
 * Optimised ANSI C code for the Rijndael cipher (now AES)
 *
 * @author Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
 * @author Antoon Bosselaers <antoon.bosselaers@esat.kuleuven.ac.be>
 * @author Paulo Barreto <paulo.barreto@terra.com.br>
 *
 * This code is hereby placed in the public domain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __RIJNDAEL_ALG_FST_H
#define __RIJNDAEL_ALG_FST_H

#include "tunsafe_types.h"

#define AESGCM_MAXNR	14

struct AesContext {
  uint32 rk[(AESGCM_MAXNR + 1) * 4];
  int rounds;
};

typedef struct { uint64 hi, lo; } aesgcm_u128;

struct AesGcm128StaticContext {
  void(*gmult)(uint64 Xi[2], const aesgcm_u128 Htable[16]);
  void(*ghash)(uint64 Xi[2], const aesgcm_u128 Htable[16], const uint8 *inp, size_t len);
  bool use_aesni_gcm_crypt;

  // Don't move H and Htable cause the asm code depends on them
  union { uint64 u[2]; uint32 d[4]; uint8 c[16]; size_t t[16 / sizeof(size_t)]; } H;
  aesgcm_u128 Htable[16];
  AesContext aes;
};

struct AesGcm128TempContext {
  AesGcm128StaticContext *sctx;
  union { uint64 u[2]; uint32 d[4]; uint8 c[16]; size_t t[16/sizeof(size_t)]; } EKi,EK0,len, Yi, Xi;
  unsigned int mres, ares;
};

void CRYPTO_gcm128_init(AesGcm128StaticContext *ctx, const uint8 *key, int key_size);

void CRYPTO_gcm128_setiv(AesGcm128TempContext *ctx, AesGcm128StaticContext *sctx, const unsigned char *iv,size_t len);
void CRYPTO_gcm128_aad(AesGcm128TempContext *ctx,const uint8 *aad,size_t len);
void CRYPTO_gcm128_encrypt_ctr32(AesGcm128TempContext *ctx, const uint8 *in, uint8 *out, size_t len);
void CRYPTO_gcm128_decrypt_ctr32(AesGcm128TempContext *ctx, const uint8 *in, uint8 *out, size_t len);
void CRYPTO_gcm128_finish(AesGcm128TempContext *ctx, unsigned char *tag, size_t len);

void aesgcm_encrypt(uint8 *dst, const uint8 *src, const size_t src_len,
                    const uint8 *ad, const size_t ad_len,
                    const uint64 nonce, AesGcm128StaticContext *sctx);

void aesgcm_decrypt_get_mac(uint8 *dst, const uint8 *src, const size_t src_len,
                            const uint8 *ad, const size_t ad_len,
                            const uint64 nonce, AesGcm128StaticContext *sctx,
                            uint8 mac[16]);

#if defined(ARCH_CPU_X86_64)
#define WITH_AESGCM 0
#endif



#endif /* __RIJNDAEL_ALG_FST_H */
