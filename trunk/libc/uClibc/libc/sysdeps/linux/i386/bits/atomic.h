/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

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


#ifndef LOCK_PREFIX
# ifdef UP
#  define LOCK_PREFIX	/* nothing */
# else
#  define LOCK_PREFIX "lock;"
# endif
#endif


#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile (LOCK_PREFIX "cmpxchgb %b2, %1"			      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "q" (newval), "m" (*mem), "0" (oldval));	      \
     ret; })

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile (LOCK_PREFIX "cmpxchgw %w2, %1"			      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "r" (newval), "m" (*mem), "0" (oldval));	      \
     ret; })

#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile (LOCK_PREFIX "cmpxchgl %2, %1"			      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "r" (newval), "m" (*mem), "0" (oldval));	      \
     ret; })

/* XXX We do not really need 64-bit compare-and-exchange.  At least
   not in the moment.  Using it would mean causing portability
   problems since not many other 32-bit architectures have support for
   such an operation.  So don't define any code for now.  If it is
   really going to be used the code below can be used on Intel Pentium
   and later, but NOT on i486.  */
#if 1
# define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret = *(mem); abort (); ret = (newval); ret = (oldval); })
#else
# ifdef __PIC__
#  define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile ("xchgl %2, %%ebx\n\t"				      \
		       LOCK_PREFIX "cmpxchg8b %1\n\t"			      \
		       "xchgl %2, %%ebx"				      \
		       : "=A" (ret), "=m" (*mem)			      \
		       : "DS" (((unsigned long long int) (newval))	      \
			       & 0xffffffff),				      \
			 "c" (((unsigned long long int) (newval)) >> 32),     \
			 "m" (*mem), "a" (((unsigned long long int) (oldval)) \
					  & 0xffffffff),		      \
			 "d" (((unsigned long long int) (oldval)) >> 32));    \
     ret; })
# else
#  define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile (LOCK_PREFIX "cmpxchg8b %1"			      \
		       : "=A" (ret), "=m" (*mem)			      \
		       : "b" (((unsigned long long int) (newval))	      \
			      & 0xffffffff),				      \
			 "c" (((unsigned long long int) (newval)) >> 32),     \
			 "m" (*mem), "a" (((unsigned long long int) (oldval)) \
					  & 0xffffffff),		      \
			 "d" (((unsigned long long int) (oldval)) >> 32));    \
     ret; })
# endif
#endif


/* Note that we need no lock prefix.  */
#define atomic_exchange_acq(mem, newvalue) \
  ({ __typeof (*mem) result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile ("xchgb %b0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "m" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile ("xchgw %w0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "m" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile ("xchgl %0, %1"					      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "m" (*mem));			      \
     else								      \
       {								      \
	 result = 0;							      \
	 abort ();							      \
       }								      \
     result; })


#define atomic_exchange_and_add(mem, value) \
  ({ __typeof (*mem) __result;						      \
     __typeof (value) __addval = (value);				      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK_PREFIX "xaddb %b0, %1"			      \
			 : "=r" (__result), "=m" (*mem)			      \
			 : "0" (__addval), "m" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK_PREFIX "xaddw %w0, %1"			      \
			 : "=r" (__result), "=m" (*mem)			      \
			 : "0" (__addval), "m" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK_PREFIX "xaddl %0, %1"			      \
			 : "=r" (__result), "=m" (*mem)			      \
			 : "0" (__addval), "m" (*mem));			      \
     else								      \
       {								      \
	 __typeof (mem) __memp = (mem);					      \
	 __typeof (*mem) __tmpval;					      \
	 __result = *__memp;						      \
	 do								      \
	   __tmpval = __result;						      \
	 while ((__result = __arch_compare_and_exchange_val_64_acq	      \
		 (__memp, __result + __addval, __result)) == __tmpval);	      \
       }								      \
     __result; })


#define atomic_add(mem, value) \
  (void) ({ if (__builtin_constant_p (value) && (value) == 1)		      \
	      atomic_increment (mem);					      \
	    else if (__builtin_constant_p (value) && (value) == -1)	      \
	      atomic_decrement (mem);					      \
	    else if (sizeof (*mem) == 1)				      \
	      __asm __volatile (LOCK_PREFIX "addb %b1, %0"		      \
				: "=m" (*mem)				      \
				: "ir" (value), "m" (*mem));		      \
	    else if (sizeof (*mem) == 2)				      \
	      __asm __volatile (LOCK_PREFIX "addw %w1, %0"		      \
				: "=m" (*mem)				      \
				: "ir" (value), "m" (*mem));		      \
	    else if (sizeof (*mem) == 4)				      \
	      __asm __volatile (LOCK_PREFIX "addl %1, %0"		      \
				: "=m" (*mem)				      \
				: "ir" (value), "m" (*mem));		      \
	    else							      \
	      {								      \
		__typeof (value) __addval = (value);			      \
		__typeof (mem) __memp = (mem);				      \
		__typeof (*mem) __oldval = *__memp;			      \
		__typeof (*mem) __tmpval;				      \
		do							      \
		  __tmpval = __oldval;					      \
		while ((__oldval = __arch_compare_and_exchange_val_64_acq     \
		       (__memp, __oldval + __addval, __oldval)) == __tmpval); \
	      }								      \
	    })


#define atomic_add_negative(mem, value) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK_PREFIX "addb %b2, %0; sets %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "iq" (value), "m" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK_PREFIX "addw %w2, %0; sets %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "m" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK_PREFIX "addl %2, %0; sets %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "m" (*mem));			      \
     else								      \
       abort ();							      \
     __result; })


#define atomic_add_zero(mem, value) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK_PREFIX "addb %b2, %0; setz %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "m" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK_PREFIX "addw %w2, %0; setz %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "m" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK_PREFIX "addl %2, %0; setz %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "ir" (value), "m" (*mem));			      \
     else								      \
       abort ();							      \
     __result; })


#define atomic_increment(mem) \
  (void) ({ if (sizeof (*mem) == 1)					      \
	      __asm __volatile (LOCK_PREFIX "incb %b0"			      \
				: "=m" (*mem)				      \
				: "m" (*mem));				      \
	    else if (sizeof (*mem) == 2)				      \
	      __asm __volatile (LOCK_PREFIX "incw %w0"			      \
				: "=m" (*mem)				      \
				: "m" (*mem));				      \
	    else if (sizeof (*mem) == 4)				      \
	      __asm __volatile (LOCK_PREFIX "incl %0"			      \
				: "=m" (*mem)				      \
				: "m" (*mem));				      \
	    else							      \
	      {								      \
		__typeof (mem) __memp = (mem);				      \
		__typeof (*mem) __oldval = *__memp;			      \
		__typeof (*mem) __tmpval;				      \
		do							      \
		  __tmpval = __oldval;					      \
		while ((__oldval = __arch_compare_and_exchange_val_64_acq     \
		       (__memp, __oldval + 1, __oldval)) == __tmpval);	      \
	      }								      \
	    })


#define atomic_increment_and_test(mem) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK_PREFIX "incb %0; sete %b1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK_PREFIX "incw %0; sete %w1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK_PREFIX "incl %0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else								      \
       abort ();							      \
     __result; })


#define atomic_decrement(mem) \
  (void) ({ if (sizeof (*mem) == 1)					      \
	      __asm __volatile (LOCK_PREFIX "decb %b0"			      \
				: "=m" (*mem)				      \
				: "m" (*mem));				      \
	    else if (sizeof (*mem) == 2)				      \
	      __asm __volatile (LOCK_PREFIX "decw %w0"			      \
				: "=m" (*mem)				      \
				: "m" (*mem));				      \
	    else if (sizeof (*mem) == 4)				      \
	      __asm __volatile (LOCK_PREFIX "decl %0"			      \
				: "=m" (*mem)				      \
				: "m" (*mem));				      \
	    else							      \
	      {								      \
		__typeof (mem) __memp = (mem);				      \
		__typeof (*mem) __oldval = *__memp;			      \
		__typeof (*mem) __tmpval;				      \
		do							      \
		  __tmpval = __oldval;					      \
		while ((__oldval = __arch_compare_and_exchange_val_64_acq     \
		       (__memp, __oldval - 1, __oldval)) == __tmpval); 	      \
	      }								      \
	    })


#define atomic_decrement_and_test(mem) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK_PREFIX "decb %b0; sete %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK_PREFIX "decw %w0; sete %1"		      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK_PREFIX "decl %0; sete %1"			      \
			 : "=m" (*mem), "=qm" (__result)		      \
			 : "m" (*mem));					      \
     else								      \
       abort ();							      \
     __result; })


#define atomic_bit_set(mem, bit) \
  (void) ({ if (sizeof (*mem) == 1)					      \
	      __asm __volatile (LOCK_PREFIX "orb %b2, %0"		      \
				: "=m" (*mem)				      \
				: "m" (*mem), "ir" (1 << (bit)));	      \
	    else if (sizeof (*mem) == 2)				      \
	      __asm __volatile (LOCK_PREFIX "orw %w2, %0"		      \
				: "=m" (*mem)				      \
				: "m" (*mem), "ir" (1 << (bit)));	      \
	    else if (sizeof (*mem) == 4)				      \
	      __asm __volatile (LOCK_PREFIX "orl %2, %0"		      \
				: "=m" (*mem)				      \
				: "m" (*mem), "ir" (1 << (bit)));	      \
	    else							      \
	      abort ();							      \
	    })


#define atomic_bit_test_set(mem, bit) \
  ({ unsigned char __result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile (LOCK_PREFIX "btsb %3, %1; setc %0"		      \
			 : "=q" (__result), "=m" (*mem)			      \
			 : "m" (*mem), "ir" (bit));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile (LOCK_PREFIX "btsw %3, %1; setc %0"		      \
			 : "=q" (__result), "=m" (*mem)			      \
			 : "m" (*mem), "ir" (bit));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile (LOCK_PREFIX "btsl %3, %1; setc %0"		      \
			 : "=q" (__result), "=m" (*mem)			      \
			 : "m" (*mem), "ir" (bit));			      \
     else							      	      \
       abort ();							      \
     __result; })


#define atomic_delay() asm ("rep; nop")
