/* Internal macros for atomic operations for GNU C Library.
   Copyright (C) 2002-2006, 2009 Free Software Foundation, Inc.
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

#ifndef _ATOMIC_H
#define _ATOMIC_H	1

/* This header defines three types of macros:

   - atomic arithmetic and logic operation on memory.  They all
     have the prefix "atomic_".

   - conditionally atomic operations of the same kinds.  These
     always behave identical but can be faster when atomicity
     is not really needed since only one thread has access to
     the memory location.  In that case the code is slower in
     the multi-thread case.  The interfaces have the prefix
     "catomic_".

   - support functions like barriers.  They also have the preifx
     "atomic_".

   Architectures must provide a few lowlevel macros (the compare
   and exchange definitions).  All others are optional.  They
   should only be provided if the architecture has specific
   support for the operation.

   As <atomic.h> macros are usually heavily nested and often use local
   variables to make sure side-effects are evaluated properly, use for
   macro local variables a per-macro unique prefix.  This file uses
   __atgN_ prefix where N is different in each macro.  */

#include <stdlib.h>

#include <bits/atomic.h>

/* Wrapper macros to call pre_NN_post (mem, ...) where NN is the
   bit width of *MEM.  The calling macro puts parens around MEM
   and following args.  */
#define __atomic_val_bysize(pre, post, mem, ...)			      \
  ({									      \
    __typeof (*mem) __atg1_result;					      \
    if (sizeof (*mem) == 1)						      \
      __atg1_result = pre##_8_##post (mem, __VA_ARGS__);		      \
    else if (sizeof (*mem) == 2)					      \
      __atg1_result = pre##_16_##post (mem, __VA_ARGS__);		      \
    else if (sizeof (*mem) == 4)					      \
      __atg1_result = pre##_32_##post (mem, __VA_ARGS__);		      \
    else if (sizeof (*mem) == 8)					      \
      __atg1_result = pre##_64_##post (mem, __VA_ARGS__);		      \
    else								      \
      abort ();								      \
    __atg1_result;							      \
  })
#define __atomic_bool_bysize(pre, post, mem, ...)			      \
  ({									      \
    int __atg2_result;							      \
    if (sizeof (*mem) == 1)						      \
      __atg2_result = pre##_8_##post (mem, __VA_ARGS__);		      \
    else if (sizeof (*mem) == 2)					      \
      __atg2_result = pre##_16_##post (mem, __VA_ARGS__);		      \
    else if (sizeof (*mem) == 4)					      \
      __atg2_result = pre##_32_##post (mem, __VA_ARGS__);		      \
    else if (sizeof (*mem) == 8)					      \
      __atg2_result = pre##_64_##post (mem, __VA_ARGS__);		      \
    else								      \
      abort ();								      \
    __atg2_result;							      \
  })


/* Atomically store NEWVAL in *MEM if *MEM is equal to OLDVAL.
   Return the old *MEM value.  */
#if !defined atomic_compare_and_exchange_val_acq \
    && defined __arch_compare_and_exchange_val_32_acq
# define atomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  __atomic_val_bysize (__arch_compare_and_exchange_val,acq,		      \
		       mem, newval, oldval)
#endif


#ifndef catomic_compare_and_exchange_val_acq
# ifdef __arch_c_compare_and_exchange_val_32_acq
#  define catomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  __atomic_val_bysize (__arch_c_compare_and_exchange_val,acq,		      \
		       mem, newval, oldval)
# else
#  define catomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  atomic_compare_and_exchange_val_acq (mem, newval, oldval)
# endif
#endif


#ifndef catomic_compare_and_exchange_val_rel
# ifndef atomic_compare_and_exchange_val_rel
#  define catomic_compare_and_exchange_val_rel(mem, newval, oldval)	      \
  catomic_compare_and_exchange_val_acq (mem, newval, oldval)
# else
#  define catomic_compare_and_exchange_val_rel(mem, newval, oldval)	      \
  atomic_compare_and_exchange_val_rel (mem, newval, oldval)
# endif
#endif


#ifndef atomic_compare_and_exchange_val_rel
# define atomic_compare_and_exchange_val_rel(mem, newval, oldval)	      \
  atomic_compare_and_exchange_val_acq (mem, newval, oldval)
#endif


/* Atomically store NEWVAL in *MEM if *MEM is equal to OLDVAL.
   Return zero if *MEM was changed or non-zero if no exchange happened.  */
#ifndef atomic_compare_and_exchange_bool_acq
# ifdef __arch_compare_and_exchange_bool_32_acq
#  define atomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
  __atomic_bool_bysize (__arch_compare_and_exchange_bool,acq,		      \
		        mem, newval, oldval)
# else
#  define atomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
  ({ /* Cannot use __oldval here, because macros later in this file might     \
	call this macro with __oldval argument.	 */			      \
     __typeof (oldval) __atg3_old = (oldval);				      \
     atomic_compare_and_exchange_val_acq (mem, newval, __atg3_old)	      \
       != __atg3_old;							      \
  })
# endif
#endif


#ifndef catomic_compare_and_exchange_bool_acq
# ifdef __arch_c_compare_and_exchange_bool_32_acq
#  define catomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
  __atomic_bool_bysize (__arch_c_compare_and_exchange_bool,acq,		      \
		        mem, newval, oldval)
# else
#  define catomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
  ({ /* Cannot use __oldval here, because macros later in this file might     \
	call this macro with __oldval argument.	 */			      \
     __typeof (oldval) __atg4_old = (oldval);				      \
     catomic_compare_and_exchange_val_acq (mem, newval, __atg4_old)	      \
       != __atg4_old;							      \
  })
# endif
#endif


#ifndef catomic_compare_and_exchange_bool_rel
# ifndef atomic_compare_and_exchange_bool_rel
#  define catomic_compare_and_exchange_bool_rel(mem, newval, oldval)	      \
  catomic_compare_and_exchange_bool_acq (mem, newval, oldval)
# else
#  define catomic_compare_and_exchange_bool_rel(mem, newval, oldval)	      \
  atomic_compare_and_exchange_bool_rel (mem, newval, oldval)
# endif
#endif


#ifndef atomic_compare_and_exchange_bool_rel
# define atomic_compare_and_exchange_bool_rel(mem, newval, oldval) \
  atomic_compare_and_exchange_bool_acq (mem, newval, oldval)
#endif


/* Store NEWVALUE in *MEM and return the old value.  */
#ifndef atomic_exchange_acq
# define atomic_exchange_acq(mem, newvalue) \
  ({ __typeof (*(mem)) __atg5_oldval;					      \
     __typeof (mem) __atg5_memp = (mem);				      \
     __typeof (*(mem)) __atg5_value = (newvalue);			      \
									      \
     do									      \
       __atg5_oldval = *__atg5_memp;					      \
     while (__builtin_expect						      \
	    (atomic_compare_and_exchange_bool_acq (__atg5_memp, __atg5_value, \
						   __atg5_oldval), 0));	      \
									      \
     __atg5_oldval; })
#endif

#ifndef atomic_exchange_rel
# define atomic_exchange_rel(mem, newvalue) atomic_exchange_acq (mem, newvalue)
#endif


/* Add VALUE to *MEM and return the old value of *MEM.  */
#ifndef atomic_exchange_and_add
# define atomic_exchange_and_add(mem, value) \
  ({ __typeof (*(mem)) __atg6_oldval;					      \
     __typeof (mem) __atg6_memp = (mem);				      \
     __typeof (*(mem)) __atg6_value = (value);				      \
									      \
     do									      \
       __atg6_oldval = *__atg6_memp;					      \
     while (__builtin_expect						      \
	    (atomic_compare_and_exchange_bool_acq (__atg6_memp,		      \
						   __atg6_oldval	      \
						   + __atg6_value,	      \
						   __atg6_oldval), 0));	      \
									      \
     __atg6_oldval; })
#endif


#ifndef catomic_exchange_and_add
# define catomic_exchange_and_add(mem, value) \
  ({ __typeof (*(mem)) __atg7_oldv;					      \
     __typeof (mem) __atg7_memp = (mem);				      \
     __typeof (*(mem)) __atg7_value = (value);				      \
									      \
     do									      \
       __atg7_oldv = *__atg7_memp;					      \
     while (__builtin_expect						      \
	    (catomic_compare_and_exchange_bool_acq (__atg7_memp,	      \
						    __atg7_oldv		      \
						    + __atg7_value,	      \
						    __atg7_oldv), 0));	      \
									      \
     __atg7_oldv; })
#endif


#ifndef atomic_max
# define atomic_max(mem, value) \
  do {									      \
    __typeof (*(mem)) __atg8_oldval;					      \
    __typeof (mem) __atg8_memp = (mem);					      \
    __typeof (*(mem)) __atg8_value = (value);				      \
    do {								      \
      __atg8_oldval = *__atg8_memp;					      \
      if (__atg8_oldval >= __atg8_value)				      \
	break;								      \
    } while (__builtin_expect						      \
	     (atomic_compare_and_exchange_bool_acq (__atg8_memp, __atg8_value,\
						    __atg8_oldval), 0));      \
  } while (0)
#endif


#ifndef catomic_max
# define catomic_max(mem, value) \
  do {									      \
    __typeof (*(mem)) __atg9_oldv;					      \
    __typeof (mem) __atg9_memp = (mem);					      \
    __typeof (*(mem)) __atg9_value = (value);				      \
    do {								      \
      __atg9_oldv = *__atg9_memp;					      \
      if (__atg9_oldv >= __atg9_value)					      \
	break;								      \
    } while (__builtin_expect						      \
	     (catomic_compare_and_exchange_bool_acq (__atg9_memp,	      \
						     __atg9_value,	      \
						     __atg9_oldv), 0));	      \
  } while (0)
#endif


#ifndef atomic_min
# define atomic_min(mem, value) \
  do {									      \
    __typeof (*(mem)) __atg10_oldval;					      \
    __typeof (mem) __atg10_memp = (mem);				      \
    __typeof (*(mem)) __atg10_value = (value);				      \
    do {								      \
      __atg10_oldval = *__atg10_memp;					      \
      if (__atg10_oldval <= __atg10_value)				      \
	break;								      \
    } while (__builtin_expect						      \
	     (atomic_compare_and_exchange_bool_acq (__atg10_memp,	      \
						    __atg10_value,	      \
						    __atg10_oldval), 0));     \
  } while (0)
#endif


#ifndef atomic_add
# define atomic_add(mem, value) (void) atomic_exchange_and_add ((mem), (value))
#endif


#ifndef catomic_add
# define catomic_add(mem, value) \
  (void) catomic_exchange_and_add ((mem), (value))
#endif


#ifndef atomic_increment
# define atomic_increment(mem) atomic_add ((mem), 1)
#endif


#ifndef catomic_increment
# define catomic_increment(mem) catomic_add ((mem), 1)
#endif


#ifndef atomic_increment_val
# define atomic_increment_val(mem) (atomic_exchange_and_add ((mem), 1) + 1)
#endif


#ifndef catomic_increment_val
# define catomic_increment_val(mem) (catomic_exchange_and_add ((mem), 1) + 1)
#endif


/* Add one to *MEM and return true iff it's now zero.  */
#ifndef atomic_increment_and_test
# define atomic_increment_and_test(mem) \
  (atomic_exchange_and_add ((mem), 1) + 1 == 0)
#endif


#ifndef atomic_decrement
# define atomic_decrement(mem) atomic_add ((mem), -1)
#endif


#ifndef catomic_decrement
# define catomic_decrement(mem) catomic_add ((mem), -1)
#endif


#ifndef atomic_decrement_val
# define atomic_decrement_val(mem) (atomic_exchange_and_add ((mem), -1) - 1)
#endif


#ifndef catomic_decrement_val
# define catomic_decrement_val(mem) (catomic_exchange_and_add ((mem), -1) - 1)
#endif


/* Subtract 1 from *MEM and return true iff it's now zero.  */
#ifndef atomic_decrement_and_test
# define atomic_decrement_and_test(mem) \
  (atomic_exchange_and_add ((mem), -1) == 1)
#endif


/* Decrement *MEM if it is > 0, and return the old value.  */
#ifndef atomic_decrement_if_positive
# define atomic_decrement_if_positive(mem) \
  ({ __typeof (*(mem)) __atg11_oldval;					      \
     __typeof (mem) __atg11_memp = (mem);				      \
									      \
     do									      \
       {								      \
	 __atg11_oldval = *__atg11_memp;				      \
	 if (__builtin_expect (__atg11_oldval <= 0, 0))			      \
	   break;							      \
       }								      \
     while (__builtin_expect						      \
	    (atomic_compare_and_exchange_bool_acq (__atg11_memp,	      \
						   __atg11_oldval - 1,	      \
						   __atg11_oldval), 0));      \
     __atg11_oldval; })
#endif


#ifndef atomic_add_negative
# define atomic_add_negative(mem, value)				      \
  ({ __typeof (value) __atg12_value = (value);				      \
     atomic_exchange_and_add (mem, __atg12_value) < -__atg12_value; })
#endif


#ifndef atomic_add_zero
# define atomic_add_zero(mem, value)					      \
  ({ __typeof (value) __atg13_value = (value);				      \
     atomic_exchange_and_add (mem, __atg13_value) == -__atg13_value; })
#endif


#ifndef atomic_bit_set
# define atomic_bit_set(mem, bit) \
  (void) atomic_bit_test_set(mem, bit)
#endif


#ifndef atomic_bit_test_set
# define atomic_bit_test_set(mem, bit) \
  ({ __typeof (*(mem)) __atg14_old;					      \
     __typeof (mem) __atg14_memp = (mem);				      \
     __typeof (*(mem)) __atg14_mask = ((__typeof (*(mem))) 1 << (bit));	      \
									      \
     do									      \
       __atg14_old = (*__atg14_memp);					      \
     while (__builtin_expect						      \
	    (atomic_compare_and_exchange_bool_acq (__atg14_memp,	      \
						   __atg14_old | __atg14_mask,\
						   __atg14_old), 0));	      \
									      \
     __atg14_old & __atg14_mask; })
#endif

/* Atomically *mem &= mask.  */
#ifndef atomic_and
# define atomic_and(mem, mask) \
  do {									      \
    __typeof (*(mem)) __atg15_old;					      \
    __typeof (mem) __atg15_memp = (mem);				      \
    __typeof (*(mem)) __atg15_mask = (mask);				      \
									      \
    do									      \
      __atg15_old = (*__atg15_memp);					      \
    while (__builtin_expect						      \
	   (atomic_compare_and_exchange_bool_acq (__atg15_memp,		      \
						  __atg15_old & __atg15_mask, \
						  __atg15_old), 0));	      \
  } while (0)
#endif

#ifndef catomic_and
# define catomic_and(mem, mask) \
  do {									      \
    __typeof (*(mem)) __atg20_old;					      \
    __typeof (mem) __atg20_memp = (mem);				      \
    __typeof (*(mem)) __atg20_mask = (mask);				      \
									      \
    do									      \
      __atg20_old = (*__atg20_memp);					      \
    while (__builtin_expect						      \
	   (catomic_compare_and_exchange_bool_acq (__atg20_memp,	      \
						   __atg20_old & __atg20_mask,\
						   __atg20_old), 0));	      \
  } while (0)
#endif

/* Atomically *mem &= mask and return the old value of *mem.  */
#ifndef atomic_and_val
# define atomic_and_val(mem, mask) \
  ({ __typeof (*(mem)) __atg16_old;					      \
     __typeof (mem) __atg16_memp = (mem);				      \
     __typeof (*(mem)) __atg16_mask = (mask);				      \
									      \
     do									      \
       __atg16_old = (*__atg16_memp);					      \
     while (__builtin_expect						      \
	    (atomic_compare_and_exchange_bool_acq (__atg16_memp,	      \
						   __atg16_old & __atg16_mask,\
						   __atg16_old), 0));	      \
									      \
     __atg16_old; })
#endif

/* Atomically *mem |= mask and return the old value of *mem.  */
#ifndef atomic_or
# define atomic_or(mem, mask) \
  do {									      \
    __typeof (*(mem)) __atg17_old;					      \
    __typeof (mem) __atg17_memp = (mem);				      \
    __typeof (*(mem)) __atg17_mask = (mask);				      \
									      \
    do									      \
      __atg17_old = (*__atg17_memp);					      \
    while (__builtin_expect						      \
	   (atomic_compare_and_exchange_bool_acq (__atg17_memp,		      \
						  __atg17_old | __atg17_mask, \
						  __atg17_old), 0));	      \
  } while (0)
#endif

#ifndef catomic_or
# define catomic_or(mem, mask) \
  do {									      \
    __typeof (*(mem)) __atg18_old;					      \
    __typeof (mem) __atg18_memp = (mem);				      \
    __typeof (*(mem)) __atg18_mask = (mask);				      \
									      \
    do									      \
      __atg18_old = (*__atg18_memp);					      \
    while (__builtin_expect						      \
	   (catomic_compare_and_exchange_bool_acq (__atg18_memp,	      \
						   __atg18_old | __atg18_mask,\
						   __atg18_old), 0));	      \
  } while (0)
#endif

/* Atomically *mem |= mask and return the old value of *mem.  */
#ifndef atomic_or_val
# define atomic_or_val(mem, mask) \
  ({ __typeof (*(mem)) __atg19_old;					      \
     __typeof (mem) __atg19_memp = (mem);				      \
     __typeof (*(mem)) __atg19_mask = (mask);				      \
									      \
     do									      \
       __atg19_old = (*__atg19_memp);					      \
     while (__builtin_expect						      \
	    (atomic_compare_and_exchange_bool_acq (__atg19_memp,	      \
						   __atg19_old | __atg19_mask,\
						   __atg19_old), 0));	      \
									      \
     __atg19_old; })
#endif

#ifndef atomic_full_barrier
# define atomic_full_barrier() __asm__ ("" ::: "memory")
#endif


#ifndef atomic_read_barrier
# define atomic_read_barrier() atomic_full_barrier ()
#endif


#ifndef atomic_write_barrier
# define atomic_write_barrier() atomic_full_barrier ()
#endif


#ifndef atomic_forced_read
# define atomic_forced_read(x) \
  ({ __typeof (x) __x; __asm__ ("" : "=r" (__x) : "0" (x)); __x; })
#endif


#ifndef atomic_delay
# define atomic_delay() do { /* nothing */ } while (0)
#endif

#endif	/* atomic.h */
