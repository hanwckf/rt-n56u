/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
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

void __arc_link_error (void);

#ifdef __A7__
#define atomic_full_barrier() __asm__ __volatile__("": : :"memory")
#else
#define atomic_full_barrier() __asm__ __volatile__("dmb 3": : :"memory")
#endif

/* Atomic compare and exchange. */

#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __arc_link_error (); oldval; })

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __arc_link_error (); oldval; })

#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval)	\
  ({ __arc_link_error (); oldval; })

#ifdef __CONFIG_ARC_HAS_ATOMICS__

#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval)     \
  ({									\
	__typeof(oldval) prev;						\
									\
	__asm__ __volatile__(						\
	"1:	llock   %0, [%1]	\n"				\
	"	brne    %0, %2, 2f	\n"				\
	"	scond   %3, [%1]	\n"				\
	"	bnz     1b		\n"				\
	"2:				\n"				\
	: "=&r"(prev)							\
	: "r"(mem), "ir"(oldval),					\
	  "r"(newval) /* can't be "ir". scond can't take limm for "b" */\
	: "cc", "memory");						\
									\
	prev;								\
  })

#else

#ifndef __NR_arc_usr_cmpxchg
#error "__NR_arc_usr_cmpxchg missing: Please upgrade to kernel 4.9+ headers"
#endif

/* With lack of hardware assist, use kernel to do the atomic operation
   This will only work in a UP configuration
 */
#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval)     \
  ({									\
	/* opecode INTERNAL_SYSCALL as it lacks cc clobber */		\
	register int __ret __asm__("r0") = (int)(mem);			\
	register int __a1 __asm__("r1") = (int)(oldval);		\
	register int __a2 __asm__("r2") = (int)(newval);		\
	register int _sys_num __asm__("r8") = __NR_arc_usr_cmpxchg;	\
									\
        __asm__ volatile (						\
		ARC_TRAP_INSN						\
		: "+r" (__ret)						\
		: "r"(_sys_num), "r"(__ret), "r"(__a1), "r"(__a2)	\
		: "memory", "cc");					\
									\
	/* syscall returns previous value */				\
	/* Z bit is set if cmpxchg succeeded (we don't use that yet) */	\
									\
	(__typeof(oldval)) __ret;					\
  })

#endif

/* Store NEWVALUE in *MEM and return the old value.
   Atomic EX is present in all configurations
 */

#define __arch_exchange_32_acq(mem, newval)				\
  ({									\
	__typeof__(*(mem)) val = newval;				\
									\
	__asm__ __volatile__(						\
	"ex %0, [%1]"							\
	: "+r" (val)							\
	: "r" (mem)							\
	: "memory" );							\
									\
	val;								\
  })

#define atomic_exchange_acq(mem, newval)				\
  ({									\
	if (sizeof(*(mem)) != 4)					\
		abort();						\
	__arch_exchange_32_acq(mem, newval);				\
  })
