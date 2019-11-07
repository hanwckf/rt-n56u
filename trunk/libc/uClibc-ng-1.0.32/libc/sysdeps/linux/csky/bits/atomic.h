/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifndef __CSKY_ATOMIC_H_
#define __CSKY_ATOMIC_H_

#include <stdint.h>
#include <sysdep.h>

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

# define __arch_compare_and_exchange_bool_8_int(mem, newval, oldval) \
  (abort (), 0)

# define __arch_compare_and_exchange_bool_16_int(mem, newval, oldval) \
  (abort (), 0)

# define __arch_compare_and_exchange_bool_32_int(mem, newval, oldval) \
  ({ __typeof(mem) _mem = (mem);                       \
    __typeof(oldval) _oldval = oldval;                 \
    __typeof(newval) _newval = newval;                 \
    register __typeof(oldval) _a0 __asm__ ("a0") = _oldval;        \
    register __typeof(newval) _a1 __asm__ ("a1") = _newval;        \
    register __typeof(mem) _a2 __asm__ ("a2") = _mem;          \
    __asm__ __volatile__ ("trap   2;"                     \
     : "+r" (_a0) : "r" (_a1) , "r" (_a2)                 \
     : "a3", "memory");                \
    _a0; })

#  define __arch_compare_and_exchange_bool_64_int(mem, newval, oldval) \
  (abort (), 0)

#define __arch_compare_and_exchange_val_8_int(mem, newval, oldval) \
	(abort (), 0)

#define __arch_compare_and_exchange_val_16_int(mem, newval, oldval) \
	(abort (), 0)

#define __arch_compare_and_exchange_val_32_int(mem, newval, oldval) \
  ({ __typeof (mem) _mem = (mem);                       \
	__typeof (*mem) __gret = *_mem;                     \
    unsigned int _tmp = 0;                                  \
    __typeof (oldval) _oldval = oldval;                 \
    __typeof (newval) _newval = newval;                 \
    register __typeof (oldval) _a0 __asm__ ("a0") = _oldval;    \
    register __typeof (newval) _a1 __asm__ ("a1") = _newval;    \
    register __typeof (mem) _a2 __asm__ ("a2") = _mem;          \
        __asm__ __volatile__ ("1:\n\t"        \
      "ldw      %1, (%4, 0x0)\n\t"            \
      "cmpne    %1, %0\n\t"                   \
      "bt       2f\n\t"                       \
      "mov      %2, %0\n\t"                   \
      "trap     2\n\t"                        \
      "cmpnei   %0, 0\n\t"                    \
      "mov      %0, %2\n\t"                   \
      "bt       1b\n\t"                       \
      "2:         \n\t"                       \
     :"+r" (_a0), "+r"(__gret), "+r" (_tmp) :"r" (_a1) , "r" (_a2)    \
     : "a3", "memory");                \
    __gret; })

# define __arch_compare_and_exchange_val_64_int(mem, newval, oldval) \
	(abort (), 0)

# define atomic_compare_and_exchange_bool_acq(mem, new, old)    \
  __atomic_bool_bysize (__arch_compare_and_exchange_bool, int,  \
            mem, new, old)

# define atomic_compare_and_exchange_val_acq(mem, new, old) \
  __atomic_val_bysize (__arch_compare_and_exchange_val, int,    \
               mem, new, old)

#endif
