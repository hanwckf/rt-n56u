/*
   BLAKE2 reference source code package - optimized C implementations

   Copyright 2012, Samuel Neves <sneves@dei.uc.pt>.  You may use this under the
   terms of the CC0, the OpenSSL Licence, or the Apache Public License 2.0, at
   your option.  The terms of these licenses can be found at:

   - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
   - OpenSSL license   : https://www.openssl.org/source/license.html
   - Apache 2.0        : http://www.apache.org/licenses/LICENSE-2.0

   More information about the BLAKE2 hash function can be found at
   https://blake2.net.
*/

#include <emmintrin.h>
#if defined(HAVE_SSSE3)
#include <tmmintrin.h>
#endif
#if defined(HAVE_SSE41)
#include <smmintrin.h>
#endif
#if defined(HAVE_AVX)
#include <immintrin.h>
#endif
#if defined(HAVE_XOP)
#include <x86intrin.h>
#endif

#include "blake2s-round.h"

void blake2s_compress_sse( blake2s_state *S, const uint8_t block[BLAKE2S_BLOCKBYTES] ) {
  __m128i row1, row2, row3, row4;
  __m128i buf1, buf2, buf3, buf4;
#if defined(HAVE_SSE41)
  __m128i t0, t1;
#if !defined(HAVE_XOP)
  __m128i t2;
#endif
#endif
  __m128i ff0, ff1;
#if defined(HAVE_SSSE3) && !defined(HAVE_XOP)
  const __m128i r8 = _mm_set_epi8( 12, 15, 14, 13, 8, 11, 10, 9, 4, 7, 6, 5, 0, 3, 2, 1 );
  const __m128i r16 = _mm_set_epi8( 13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2 );
#endif
#if defined(HAVE_SSE41)
  const __m128i m0 = LOADU( block +  00 );
  const __m128i m1 = LOADU( block +  16 );
  const __m128i m2 = LOADU( block +  32 );
  const __m128i m3 = LOADU( block +  48 );
#else
  const uint32_t  m0 = load32(block +  0 * sizeof(uint32_t));
  const uint32_t  m1 = load32(block +  1 * sizeof(uint32_t));
  const uint32_t  m2 = load32(block +  2 * sizeof(uint32_t));
  const uint32_t  m3 = load32(block +  3 * sizeof(uint32_t));
  const uint32_t  m4 = load32(block +  4 * sizeof(uint32_t));
  const uint32_t  m5 = load32(block +  5 * sizeof(uint32_t));
  const uint32_t  m6 = load32(block +  6 * sizeof(uint32_t));
  const uint32_t  m7 = load32(block +  7 * sizeof(uint32_t));
  const uint32_t  m8 = load32(block +  8 * sizeof(uint32_t));
  const uint32_t  m9 = load32(block +  9 * sizeof(uint32_t));
  const uint32_t m10 = load32(block + 10 * sizeof(uint32_t));
  const uint32_t m11 = load32(block + 11 * sizeof(uint32_t));
  const uint32_t m12 = load32(block + 12 * sizeof(uint32_t));
  const uint32_t m13 = load32(block + 13 * sizeof(uint32_t));
  const uint32_t m14 = load32(block + 14 * sizeof(uint32_t));
  const uint32_t m15 = load32(block + 15 * sizeof(uint32_t));
#endif
  row1 = ff0 = LOADU( &S->h[0] );
  row2 = ff1 = LOADU( &S->h[4] );
  row3 = _mm_loadu_si128( (__m128i const *)&blake2s_IV[0] );
  row4 = _mm_xor_si128( _mm_loadu_si128( (__m128i const *)&blake2s_IV[4] ), LOADU( &S->t[0] ) );
  BLAKE2S_ROUND_SSE( 0 );
  BLAKE2S_ROUND_SSE( 1 );
  BLAKE2S_ROUND_SSE( 2 );
  BLAKE2S_ROUND_SSE( 3 );
  BLAKE2S_ROUND_SSE( 4 );
  BLAKE2S_ROUND_SSE( 5 );
  BLAKE2S_ROUND_SSE( 6 );
  BLAKE2S_ROUND_SSE( 7 );
  BLAKE2S_ROUND_SSE( 8 );
  BLAKE2S_ROUND_SSE( 9 );
  STOREU( &S->h[0], _mm_xor_si128( ff0, _mm_xor_si128( row1, row3 ) ) );
  STOREU( &S->h[4], _mm_xor_si128( ff1, _mm_xor_si128( row2, row4 ) ) );
}
