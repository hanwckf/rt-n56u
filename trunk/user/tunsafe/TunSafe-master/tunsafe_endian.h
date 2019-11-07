// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#ifndef TINYVPN_ENDIAN_H_
#define TINYVPN_ENDIAN_H_

#include "build_config.h"
#include "tunsafe_types.h"
#if defined(OS_WIN) && defined(COMPILER_MSVC)
#include <intrin.h>
#endif
#include <stdint.h>

#if defined(OS_WIN) && defined(COMPILER_MSVC)
#define ByteSwap16(x) _byteswap_ushort((uint16)x)
#define ByteSwap32(x) _byteswap_ulong((uint32)x)
#define ByteSwap64(x) _byteswap_uint64((uint64)x)
#else
#define ByteSwap16(x) __builtin_bswap16((uint16)x)
#define ByteSwap32(x) __builtin_bswap32((uint32)x)
#define ByteSwap64(x) __builtin_bswap64((uint64)x)
#endif

#if defined(ARCH_CPU_LITTLE_ENDIAN)
#define ToBE64(x) ByteSwap64(x)
#define ToBE32(x) ByteSwap32(x)
#define ToBE16(x) ByteSwap16(x)
#define ToLE64(x) (x)
#define ToLE32(x) (x)
#define ToLE16(x) (x)
#elif defined(ARCH_CPU_BIG_ENDIAN)
#define ToBE64(x) (x)
#define ToBE32(x) (x)
#define ToBE16(x) (x)
#define ToLE64(x) ByteSwap64(x)
#define ToLE32(x) ByteSwap32(x)
#define ToLE16(x) ByteSwap16(x)
#else
#error The CPU is neither big / little endian
#endif

#if !(defined(COMPILER_GCC) || defined(COMPILER_CLANG) || defined(ARCH_CPU_ALLOW_UNALIGNED))
#error The CPU does not support unaligned memory accesses
#endif  // defined(ARCH_CPU_ALLOW_UNALIGNED)

#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
// The WriteBE/WriteLE functions below write a uint to a char pointer which
// is not valid per the C spec because of aliasing, so work around it.
typedef uint16  __attribute__((__may_alias__)) uint16_unaligned __attribute__((aligned(1)));
typedef uint32  __attribute__((__may_alias__)) uint32_unaligned __attribute__((aligned(1)));
typedef uint64  __attribute__((__may_alias__)) uint64_unaligned __attribute__((aligned(1)));
typedef uint16  __attribute__((__may_alias__)) uint16_aligned;
typedef uint32  __attribute__((__may_alias__)) uint32_aligned;
typedef uint64  __attribute__((__may_alias__)) uint64_aligned;
#else
typedef uint16 uint16_unaligned;
typedef uint32 uint32_unaligned;
typedef uint64 uint64_unaligned;
typedef uint16 uint16_aligned;
typedef uint32 uint32_aligned;
typedef uint64 uint64_aligned;
#endif


// Use the _Aligned variants when you are sure that the pointers are aligned
#define Read16Aligned(pt) *(uint16_aligned*)(pt)
#define Read32Aligned(pt) *(uint32_aligned*)(pt)
#define Read64Aligned(pt) *(uint64_aligned*)(pt)
#define Write16Aligned(ct, st) (*(uint16_aligned*)(ct) = (st))
#define Write32Aligned(ct, st) (*(uint32_aligned*)(ct) = (st))
#define Write64Aligned(ct, st) (*(uint64_aligned*)(ct) = (st))
#define ReadBE16Aligned(pt) ToBE16(Read16Aligned(pt))
#define ReadBE32Aligned(pt) ToBE32(Read32Aligned(pt))
#define ReadBE64Aligned(pt) ToBE64(Read64Aligned(pt))
#define WriteBE16Aligned(ct, st) Write16Aligned(ct, ToBE16(st))
#define WriteBE32Aligned(ct, st) Write32Aligned(ct, ToBE32(st))
#define WriteBE64Aligned(ct, st) Write64Aligned(ct, ToBE64(st))
#define ReadLE16Aligned(pt) ToLE16(Read16Aligned(pt))
#define ReadLE32Aligned(pt) ToLE32(Read32Aligned(pt))
#define ReadLE64Aligned(pt) ToLE64(Read64Aligned(pt))
#define WriteLE16Aligned(ct, st) Write16Aligned(ct, ToLE16(st))
#define WriteLE32Aligned(ct, st) Write32Aligned(ct, ToLE32(st))
#define WriteLE64Aligned(ct, st) Write64Aligned(ct, ToLE64(st))

// Use the below this when pointers may be unaligned
#define Read16(pt) *(uint16_unaligned*)(pt)
#define Read32(pt) *(uint32_unaligned*)(pt)
#define Read64(pt) *(uint64_unaligned*)(pt)
#define Write16(ct, st) (*(uint16_unaligned*)(ct) = (st))
#define Write32(ct, st) (*(uint32_unaligned*)(ct) = (st))
#define Write64(ct, st) (*(uint64_unaligned*)(ct) = (st))
#define ReadBE16(pt) ToBE16(Read16(pt))
#define ReadBE32(pt) ToBE32(Read32(pt))
#define ReadBE64(pt) ToBE64(Read64(pt))
#define WriteBE16(ct, st) Write16(ct, ToBE16(st))
#define WriteBE32(ct, st) Write32(ct, ToBE32(st))
#define WriteBE64(ct, st) Write64(ct, ToBE64(st))
#define ReadLE16(pt) ToLE16(Read16(pt))
#define ReadLE32(pt) ToLE32(Read32(pt))
#define ReadLE64(pt) ToLE64(Read64(pt))
#define WriteLE16(ct, st) Write16(ct, ToLE16(st))
#define WriteLE32(ct, st) Write32(ct, ToLE32(st))
#define WriteLE64(ct, st) Write64(ct, ToLE64(st))

#endif  // TINYVPN_ENDIAN_H_
