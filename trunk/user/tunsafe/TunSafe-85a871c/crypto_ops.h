// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#ifndef TUNSAFE_CRYPTO_OPS_H_
#define TUNSAFE_CRYPTO_OPS_H_

#include "build_config.h"
#include "tunsafe_types.h"

#include <string.h>

#if defined(COMPILER_MSVC)
#include <intrin.h>
#endif  // defined(COMPILER_MSVC)

#if defined(ARCH_CPU_X86_64) && defined(COMPILER_MSVC)
FORCEINLINE static void memzero_crypto(void *dst, size_t n) {
if (n & 7) {
    __stosb((unsigned char*)dst, 0, n);
  } else {
    __stosq((uint64*)dst, 0, n >> 3);
  }
}

#elif defined(ARCH_CPU_X86) && defined(COMPILER_MSVC)
FORCEINLINE static void memzero_crypto(void *dst, size_t n) {
  if (n & 3) {
    __stosb((unsigned char*)dst, 0, n);
  } else {
    __stosd((unsigned long*)dst, 0, n >> 2);
  }
}
#else
FORCEINLINE static void memzero_crypto(void *dst, size_t n) {
  memset(dst, 0, n);
  __asm__ __volatile__("": :"r"(dst) :"memory");
}
#endif

int memcmp_crypto(const uint8 *a, const uint8 *b, size_t n);


#endif  // TUNSAFE_CRYPTO_OPS_H_