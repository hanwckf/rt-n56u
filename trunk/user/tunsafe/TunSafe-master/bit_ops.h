// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once

#include "tunsafe_types.h"
#include "tunsafe_endian.h"

#if !defined(ARCH_CPU_X86_64) && defined(COMPILER_MSVC)
static inline int _BitScanReverse64(unsigned long *index, uint64 x) {
  if (_BitScanReverse(index, x >> 32)) {
    (*index) += 32;
    return true;
  }
  return _BitScanReverse(index, (uint32)x);
}
#endif

#if !defined(COMPILER_MSVC)
static inline int _BitScanReverse64(unsigned long *index, uint64 x) {
  *index = 63 - __builtin_clzll(x);
  return (x != 0);
}

static inline int _BitScanReverse(unsigned long *index, uint32 x) {
  *index = 31 - __builtin_clz(x);
  return (x != 0);
}

#endif

static inline int FindHighestSetBit32(uint32 x) {
  unsigned long index;
  return _BitScanReverse(&index, x) ? (int)(index + 1) : 0;
}

static inline int FindLastSetBit32(uint32 x) {
  unsigned long index;
  _BitScanReverse(&index, x);
  return index;
}

static inline int FindHighestSetBit64(uint64 x) {
  unsigned long index;
  return _BitScanReverse64(&index, x) ? (int)(index + 1) : 0;
}

static inline int FindHighestSetBit128(uint64 hi, uint64 lo) {
  return hi ? 64 + FindHighestSetBit64(hi) : FindHighestSetBit64(lo);
}
