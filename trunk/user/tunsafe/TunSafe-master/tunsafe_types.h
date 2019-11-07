// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#ifndef TINYVPN_TYPES_H_
#define TINYVPN_TYPES_H_
#include <stdint.h>

#include "build_config.h"
#include "tunsafe_config.h"

typedef uint8_t byte;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int64_t int64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef unsigned int uint;

#define CTASTR2(pre,post) pre ## post
#define CTASTR(pre,post) CTASTR2(pre,post)
#define STATIC_ASSERT(cond,msg) \
    typedef struct { int CTASTR(static_assertion_failed_,msg) : !!(cond); } \
        CTASTR(static_assertion_failed_x_,msg)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

void printhex(const char *name, const void *a, size_t l);

#if defined(COMPILER_MSVC)
#define FORCEINLINE __forceinline
#define NOINLINE __declspec(noinline)
#define SAFEBUFFERS __declspec(safebuffers)
#define __aligned(x) __declspec(align(x))
#define rol32 _rotl
#define rol64 _rotl64
#elif defined(COMPILER_GCC)
#define FORCEINLINE inline __attribute__((always_inline))
#define NOINLINE
#define SAFEBUFFERS
#define _stricmp strcasecmp
#define _strdup strdup
#define _cdecl
#define __aligned(x) __attribute__((__aligned__(x))) 
#else
#define FORCEINLINE inline
#define NOINLINE
#define SAFEBUFFERS
#define __aligned(x)
#endif

#define likely(x) (x)
#define unlikely(x) (x)

#if !defined(COMPILER_MSVC)
static inline uint64 rol64(uint64 x, int8_t r) {
  return (x << r) | (x >> (64 - r));
}
static inline uint32 rol32(uint32 x, int8_t r) {
  return (x << r) | (x >> (32 - r));
}
#endif  // !defined(COMPILER_MSVC)

void RERROR(const char *msg, ...);
void RINFO(const char *msg, ...);
void tunsafe_die(const char *msg);

#ifdef _DEBUG
#define DPRINTF RINFO
#else
#define DPRINTF(msg, ...)
#endif

#endif  // TINYVPN_TYPES_H_
