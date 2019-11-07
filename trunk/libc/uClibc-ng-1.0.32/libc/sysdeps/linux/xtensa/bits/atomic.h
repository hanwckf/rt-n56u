/* Copyright (C) 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _BITS_ATOMIC_H
#define _BITS_ATOMIC_H  1

#include <inttypes.h>

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


/* Xtensa has only a 32-bit form of a store-conditional instruction.  */

#define __arch_compare_and_exchange_bool_8_acq(mem, newval, oldval) \
      (abort (), 0)

#define __arch_compare_and_exchange_bool_16_acq(mem, newval, oldval) \
      (abort (), 0)

#define __arch_compare_and_exchange_bool_8_rel(mem, newval, oldval) \
      (abort (), 0)

#define __arch_compare_and_exchange_bool_16_rel(mem, newval, oldval) \
      (abort (), 0)

/* Atomically store NEWVAL in *MEM if *MEM is equal to OLDVAL.
   Return the old *MEM value.  */

#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval)  \
  ({__typeof__(*(mem)) __tmp, __value;                               \
    __asm__ __volatile__(                                            \
      "1:     l32i    %1, %2, 0               \n"                    \
      "       bne     %1, %4, 2f              \n"                    \
      "       wsr     %1, SCOMPARE1           \n"                    \
      "       mov     %0, %1                  \n"                    \
      "       mov     %1, %3                  \n"                    \
      "       s32c1i  %1, %2, 0               \n"                    \
      "       bne     %0, %1, 1b              \n"                    \
      "2:                                     \n"                    \
      : "=&a" (__value), "=&a" (__tmp)                               \
      : "a" (mem), "a" (newval), "a" (oldval)                        \
      : "memory" );                                                  \
    __tmp;                                                           \
  })

/* Atomically store NEWVAL in *MEM if *MEM is equal to OLDVAL.
   Return zero if *MEM was changed or non-zero if no exchange happened.  */

#define __arch_compare_and_exchange_bool_32_acq(mem, newval, oldval) \
  ({__typeof__(*(mem)) __tmp, __value;                               \
    __asm__ __volatile__(                                            \
      "1:     l32i    %0, %2, 0               \n"                    \
      "       sub     %1, %4, %0              \n"                    \
      "       bnez    %1, 2f                  \n"                    \
      "       wsr     %0, SCOMPARE1           \n"                    \
      "       mov     %1, %3                  \n"                    \
      "       s32c1i  %1, %2, 0               \n"                    \
      "       bne     %0, %1, 1b              \n"                    \
      "       movi    %1, 0                   \n"                    \
      "2:                                     \n"                    \
      : "=&a" (__value), "=&a" (__tmp)                               \
      : "a" (mem), "a" (newval), "a" (oldval)                        \
      : "memory" );                                                  \
    __tmp != 0;                                                      \
  })

/* Store NEWVALUE in *MEM and return the old value.  */

#define __arch_exchange_32_acq(mem, newval)                          \
  ({__typeof__(*(mem)) __tmp, __value;                               \
    __asm__ __volatile__(                                            \
      "1:     l32i    %0, %2, 0               \n"                    \
      "       wsr     %0, SCOMPARE1           \n"                    \
      "       mov     %1, %3                  \n"                    \
      "       s32c1i  %1, %2, 0               \n"                    \
      "       bne     %0, %1, 1b              \n"                    \
      : "=&a" (__value), "=&a" (__tmp)                               \
      : "a" (mem), "a" (newval)                                      \
      : "memory" );                                                  \
    __tmp;                                                           \
  })

/* Add VALUE to *MEM and return the old value of *MEM.  */

#define __arch_atomic_exchange_and_add_32(mem, value)                \
  ({__typeof__(*(mem)) __tmp, __value;                               \
    __asm__ __volatile__(                                            \
      "1:     l32i    %0, %2, 0               \n"                    \
      "       wsr     %0, SCOMPARE1           \n"                    \
      "       add     %1, %0, %3              \n"                    \
      "       s32c1i  %1, %2, 0               \n"                    \
      "       bne     %0, %1, 1b              \n"                    \
      : "=&a" (__value), "=&a" (__tmp)                               \
      : "a" (mem), "a" (value)                                       \
      : "memory" );                                                  \
    __tmp;                                                           \
  })

/* Subtract VALUE from *MEM and return the old value of *MEM.  */

#define __arch_atomic_exchange_and_sub_32(mem, value)                \
  ({__typeof__(*(mem)) __tmp, __value;                               \
    __asm__ __volatile__(                                            \
      "1:     l32i    %0, %2, 0               \n"                    \
      "       wsr     %0, SCOMPARE1           \n"                    \
      "       sub     %1, %0, %3              \n"                    \
      "       s32c1i  %1, %2, 0               \n"                    \
      "       bne     %0, %1, 1b              \n"                    \
      : "=&a" (__value), "=&a" (__tmp)                               \
      : "a" (mem), "a" (value)                                       \
      : "memory" );                                                  \
    __tmp;                                                           \
  })

/* Decrement *MEM if it is > 0, and return the old value.  */

#define __arch_atomic_decrement_if_positive_32(mem)                  \
  ({__typeof__(*(mem)) __tmp, __value;                               \
    __asm__ __volatile__(                                            \
      "1:     l32i    %0, %2, 0               \n"                    \
      "       blti    %0, 1, 2f               \n"                    \
      "       wsr     %0, SCOMPARE1           \n"                    \
      "       addi    %1, %0, -1              \n"                    \
      "       s32c1i  %1, %2, 0               \n"                    \
      "       bne     %0, %1, 1b              \n"                    \
      "2:                                     \n"                    \
      : "=&a" (__value), "=&a" (__tmp)                               \
      : "a" (mem)                                                    \
      : "memory" );                                                  \
    __value;                                                         \
  })


/* These are the preferred public interfaces: */

#define atomic_compare_and_exchange_val_acq(mem, newval, oldval)     \
  ({                                                                 \
    if (sizeof (*mem) != 4)                                          \
      abort();                                                       \
    __arch_compare_and_exchange_val_32_acq(mem, newval, oldval);     \
  })

#define atomic_exchange_acq(mem, newval)                             \
  ({                                                                 \
    if (sizeof(*(mem)) != 4)                                         \
      abort();                                                       \
    __arch_exchange_32_acq(mem, newval);                             \
  })

#define atomic_exchange_and_add(mem, newval)                         \
  ({                                                                 \
    if (sizeof(*(mem)) != 4)                                         \
      abort();                                                       \
    __arch_atomic_exchange_and_add_32(mem, newval);                  \
  })

#define atomic_exchange_and_sub(mem, newval)                         \
  ({                                                                 \
    if (sizeof(*(mem)) != 4)                                         \
      abort();                                                       \
    __arch_atomic_exchange_and_sub_32(mem, newval);                  \
  })

#define atomic_decrement_if_positive(mem)                            \
  ({                                                                 \
    if (sizeof(*(mem)) != 4)                                         \
      abort();                                                       \
    __arch_atomic_decrement_if_positive_32(mem);                     \
  })


# define __arch_compare_and_exchange_bool_64_acq(mem, newval, oldval) \
    (abort (), 0)

# define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
    (abort (), (__typeof (*mem)) 0)

# define __arch_compare_and_exchange_bool_64_rel(mem, newval, oldval) \
    (abort (), 0)

# define __arch_compare_and_exchange_val_64_rel(mem, newval, oldval) \
    (abort (), (__typeof (*mem)) 0)

# define __arch_atomic_exchange_64_acq(mem, value) \
    ({ abort (); (*mem) = (value); })

# define __arch_atomic_exchange_64_rel(mem, value) \
    ({ abort (); (*mem) = (value); })

# define __arch_atomic_exchange_and_add_64(mem, value) \
    ({ abort (); (*mem) = (value); })

# define __arch_atomic_increment_val_64(mem) \
    ({ abort (); (*mem)++; })

# define __arch_atomic_decrement_val_64(mem) \
    ({ abort (); (*mem)--; })

# define __arch_atomic_decrement_if_positive_64(mem) \
    ({ abort (); (*mem)--; })



#endif /* _BITS_ATOMIC_H */

