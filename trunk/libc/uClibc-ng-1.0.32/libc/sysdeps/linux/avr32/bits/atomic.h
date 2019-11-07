/*
 * Copyright (C) 2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */
#ifndef _AVR32_BITS_ATOMIC_H
#define _AVR32_BITS_ATOMIC_H 1

#include <inttypes.h>

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval)	\
	(abort(), 0)

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval)	\
	(abort(), 0)

#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval)	\
	({								\
		__uint32_t __result;					\
		__typeof__(*(mem)) __prev;				\
		__asm__ __volatile__(					\
			"/* __arch_compare_and_exchange_val_32_acq */\n" \
			"1:	ssrf	5\n"				\
			"	ld.w	%[result], %[m]\n"		\
			"	cp.w	%[result], %[old]\n"		\
			"	brne	2f\n"				\
			"	stcond	%[m], %[new]\n"			\
			"	brne	1b\n"				\
			"2:"						\
			: [result] "=&r"(__result), [m] "=m"(*(mem))	\
			: "m"(*(mem)), [old] "ir"(oldval),		\
			  [new] "r"(newval)				\
			: "memory", "cc");				\
		__prev;							\
	})

#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval)	\
	(abort(), 0)

#define __arch_exchange_32_acq(mem, newval)				\
	({								\
		__typeof__(*(mem)) __oldval;				\
		__asm__ __volatile__(					\
			"/*__arch_exchange_32_acq */\n"			\
			"	xchg	%[old], %[m], %[new]"		\
			: [old] "=&r"(__oldval)				\
			: [m] "r"(mem), [new] "r"(newval)		\
			: "memory");					\
		__oldval;						\
	})

#define __arch_atomic_exchange_and_add_32(mem, value)			\
	({								\
		__typeof__(*(mem)) __oldval, __tmp;			\
		__asm__ __volatile__(					\
			"/* __arch_atomic_exchange_and_add_32 */\n"	\
			"1:	ssrf	5\n"				\
			"	ld.w	%[old], %[m]\n"			\
			"	add	%[tmp], %[old], %[val]\n"	\
			"	stcond	%[m], %[tmp]\n"			\
			"	brne	1b"				\
			: [old] "=&r"(__oldval), [tmp] "=&r"(__tmp),	\
			  [m] "=m"(*(mem))				\
			: "m"(*(mem)), [val] "r"(value)			\
			: "memory", "cc");				\
		__oldval;						\
	})

#define __arch_atomic_decrement_if_positive_32(mem)			\
	({								\
		__typeof__(*(mem)) __oldval, __tmp;			\
		__asm__ __volatile__(					\
			"/* __arch_atomic_decrement_if_positive_32 */\n" \
			"1:	ssrf	5\n"				\
			"	ld.w	%[old], %[m]\n"			\
			"	sub	%[tmp], %[old], 1\n"		\
			"	brlt	2f\n"				\
			"	stcond	%[m], %[tmp]\n"			\
			"	brne	1b"				\
			"2:"						\
			: [old] "=&r"(__oldval), [tmp] "=&r"(__tmp),	\
			  [m] "=m"(*(mem))				\
			: "m"(*(mem))					\
			: "memory", "cc");				\
		__oldval;						\
	})

#define atomic_exchange_acq(mem, newval)				\
	({								\
		if (sizeof(*(mem)) != 4)				\
			abort();					\
		__arch_exchange_32_acq(mem, newval);			\
	})

#define atomic_exchange_and_add(mem, newval)				\
	({								\
		if (sizeof(*(mem)) != 4)				\
			abort();					\
		__arch_atomic_exchange_and_add_32(mem, newval);		\
	})

#define atomic_decrement_if_positive(mem)				\
	({								\
		if (sizeof(*(mem)) != 4)				\
			abort();					\
		__arch_atomic_decrement_if_positive_32(mem);		\
	})

#endif /* _AVR32_BITS_ATOMIC_H */
