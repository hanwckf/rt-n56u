/*
 * Copyrith (C) 2013 Imagination Technologies Ltd.
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 *
 */

#include <stdint.h>
#include <sysdep.h>

typedef int8_t atomic8_t;
typedef uint8_t uatomic8_t;
typedef int_fast8_t atomic_fast8_t;
typedef uint_fast8_t uatomic_fast8_t;

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

void __metag_link_error (void);

#define atomic_full_barrier() \
     __asm__ __volatile__("": : :"memory")

/* Atomic compare and exchange.  This sequence relies on the kernel to
   provide a compare and exchange operation which is atomic. */

#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __metag_link_error (); oldval; })

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __metag_link_error (); oldval; })

/* This code uses the kernel helper to do cmpxchg. It relies on the fact
   the helper code only clobbers D0Re0. */
#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval)     \
  ({ register __typeof (oldval) a_current __asm__ ("D1Ar1");		\
    register __typeof (oldval) a_newval __asm__ ("D0Ar2") = (newval);   \
    register __typeof (mem) a_ptr __asm__ ("D1Ar3") = (mem);            \
    register __typeof (oldval) a_oldval __asm__ ("D0Ar4") = (oldval);	\
    __asm__ __volatile__						\
      ("0:\n\t"								\
       "GETD	%[cur], [%[ptr]]\n\t"					\
       "CMP	%[cur], %[old]\n\t"					\
       "BNE	1f\n\t"							\
       "MOVT	D1RtP, #0x6fff\n\t"					\
       "ADD	D1RtP, D1RtP, #0xf040\n\t"				\
       "SWAP	D1RtP, PC\n\t"						\
       "MOV	%[cur], %[old]\n\t"					\
       "CMP	D0Re0, #0\n\t"						\
       "BNE	0b\n\t"							\
       "1:"								\
       : [cur] "=&r" (a_current)					\
       : [new] "r" (a_newval), [ptr] "r" (a_ptr),			\
	 [old] "r" (a_oldval)						\
       : "D0Re0", "D1RtP", "cc", "memory");				\
    a_current; })

#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __metag_link_error (); oldval; })
