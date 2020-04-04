/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _NDS32_BITS_ATOMIC_H
#define _NDS32_BITS_ATOMIC_H

#include <stdint.h>

typedef int8_t atomic8_t;
typedef uint8_t uatomic8_t;
typedef int_fast8_t atomic_fast8_t;
typedef uint_fast8_t uatomic_fast8_t;

typedef int16_t atomic16_t;
typedef uint16_t uatomic16_t;
typedef int_fast16_t atomic_fast16_t;
typedef uint_fast16_t uatomic_fast16_t;

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef int64_t atomic64_t;
typedef uint64_t uatomic64_t;
typedef int_fast64_t atomic_fast64_t;
typedef uint_fast64_t uatomic_fast64_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;


#ifndef atomic_full_barrier
# define atomic_full_barrier() __asm__ ("dsb" ::: "memory")
#endif

#ifndef atomic_read_barrier
# define atomic_read_barrier() atomic_full_barrier ()
#endif

#ifndef atomic_write_barrier
# define atomic_write_barrier() atomic_full_barrier ()
#endif

#define atomic_exchange_acq(mem, newval)                    \
  ({   unsigned long val, offset, temp;                     \
                                                            \
       __asm__ volatile (                                       \
               "move   %2, %4\n\t"                          \
               "move   %1, #0x0\n\t"                        \
               "1:\n\t"                                     \
               "llw    %0, [%3 + %1 << 0]\n\t"              \
               "move   %2, %4\n\t"                          \
               "scw    %2, [%3 + %1 << 0]\n\t"              \
               "beqz   %2, 1b\n\t"                          \
               : "=&r" (val), "=&r" (offset), "=&r" (temp)  \
               : "r" (mem), "r" (newval)                    \
               : "memory" );                                \
       val; })

#define atomic_compare_and_exchange_val_acq(mem, newval, oldval)  \
  ({   unsigned long val, offset, temp;                     \
                                                            \
       __asm__ volatile (                                       \
               "move   %1, #0x0\n\t"                        \
               "move   %2, %4\n\t"                          \
               "1:\n\t"                                     \
               "llw    %0, [%3 + %1 << 0]\n\t"              \
               "bne    %0, %5, 2f\n\t"                      \
               "move   %2, %4\n\t"                          \
               "scw    %2, [%3 + %1 << 0]\n\t"              \
               "beqz   %2, 1b\n\t"                          \
               "j      3f\n\t"                              \
               "2:\n\t"                                     \
               "move   %2, %0\n\t"                          \
               "scw    %2, [%3 + %1 << 0]\n\t"              \
               "3:\n\t"                                     \
               : "=&r" (val), "=&r" (offset), "=&r" (temp)  \
               : "r" (mem), "r" (newval), "r" (oldval)      \
               : "memory" ); \
       val; })

#define atomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
  ({   unsigned long val, offset, temp;                     \
                                                            \
       __asm__ volatile (                                       \
               "move   %1, #0x0\n\t"                        \
               "move   %2, %4\n\t"                          \
               "1:\n\t"                                     \
               "llw    %0, [%3 + %1 << 0]\n\t"              \
               "bne    %5, %0, 2f\n\t"                      \
               "move   %2, %4\n\t"                          \
               "scw    %2, [%3 + %1 << 0]\n\t"              \
               "beqz   %2, 1b\n\t"                          \
               "addi   %0, %1, #0\n\t"                      \
               "j      3f\n\t"                              \
               "2:\n\t"                                     \
               "scw    %0, [%3 + %1 << 0]\n\t"              \
               "addi   %0, %1, #0x1\n\t"                    \
               "3:\n\t"                                     \
               : "=&r" (val), "=&r" (offset), "=&r" (temp)  \
               : "r" (mem), "r" (newval), "r" (oldval)      \
               : "memory" ); \
       val; })

#endif
