/* Copyright (C) 2010-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Maxim Kuvyrkov <maxim@codesourcery.com>, 2010.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _OR1K_BITS_ATOMIC_H
#define _OR1K_BITS_ATOMIC_H

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
# define atomic_full_barrier() __asm__ ("" ::: "memory")
#endif

#ifndef atomic_read_barrier
# define atomic_read_barrier() atomic_full_barrier ()
#endif

#ifndef atomic_write_barrier
# define atomic_write_barrier() atomic_full_barrier ()
#endif

#define atomic_exchange_acq(mem, newval)                    \
  ({   __typeof (newval) val;                               \
                                                            \
       __asm__ volatile (                                   \
     		"1:     l.lwa %0, 0(%1)		\n"	    \
		"       l.swa 0(%1), %2		\n"	    \
		"	l.bnf 1b		\n"	    \
		"	 l.nop			\n"	    \
		: "=&r"(val)				    \
		: "r"(mem), "r"(newval)			    \
		: "cc", "memory");			    \
       val; })

#define atomic_compare_and_exchange_val_acq(mem, newval, oldval)  \
  ({   __typeof (newval) val;		                    \
                                                            \
       __asm__ volatile (                                   \
       		"1:	l.lwa %0, 0(%1)		\n"	    \
		"	l.sfeq %0, %2		\n"	    \
		"	l.bnf 2f		\n"	    \
		"	 l.nop			\n"	    \
		"	l.swa 0(%1), %3		\n"	    \
		"	l.bnf 1b		\n"	    \
		"	 l.nop			\n"	    \
		"2:				\n"	    \
		: "=&r"(val)				    \
		: "r"(mem), "r"(oldval), "r"(newval)	    \
		: "cc", "memory");			    \
       val; })

#endif
