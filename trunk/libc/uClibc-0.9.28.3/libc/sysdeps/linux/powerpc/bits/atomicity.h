/* Low-level functions for atomic operations.  PowerPC version.
   Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _ATOMICITY_H
#define _ATOMICITY_H	1

#include <inttypes.h>

#if BROKEN_PPC_ASM_CR0
# define __ATOMICITY_INLINE /* nothing */
#else
# define __ATOMICITY_INLINE inline
#endif

static __ATOMICITY_INLINE int
__attribute_used__
exchange_and_add (volatile uint32_t *mem, int val)
{
  int tmp, result;
  __asm__ ("\n\
0:	lwarx	%0,0,%2	\n\
	add%I3	%1,%0,%3	\n\
	stwcx.	%1,0,%2	\n\
	bne-	0b	\n\
" : "=&b"(result), "=&r"(tmp) : "r" (mem), "Ir"(val) : "cr0", "memory");
  return result;
}

static __ATOMICITY_INLINE void
__attribute_used__
atomic_add (volatile uint32_t *mem, int val)
{
  int tmp;
  __asm__ ("\n\
0:	lwarx	%0,0,%1	\n\
	add%I2	%0,%0,%2	\n\
	stwcx.	%0,0,%1	\n\
	bne-	0b	\n\
" : "=&b"(tmp) : "r" (mem), "Ir"(val) : "cr0", "memory");
}

static __ATOMICITY_INLINE int
__attribute_used__
compare_and_swap (volatile long int *p, long int oldval, long int newval)
{
  int result;
  __asm__ ("\n\
0:	lwarx	%0,0,%1	\n\
	sub%I2c.	%0,%0,%2	\n\
	cntlzw	%0,%0	\n\
	bne-	1f	\n\
	stwcx.	%3,0,%1	\n\
	bne-	0b	\n\
1:	\n\
" : "=&b"(result) : "r"(p), "Ir"(oldval), "r"(newval) : "cr0", "memory");
  return result >> 5;
}

static __ATOMICITY_INLINE long int
__attribute_used__
always_swap (volatile long int *p, long int newval)
{
  long int result;
  __asm__ ("\n\
0:	lwarx	%0,0,%1	\n\
	stwcx.	%2,0,%1	\n\
	bne-	0b	\n\
" : "=&r"(result) : "r"(p), "r"(newval) : "cr0", "memory");
  return result;
}

static __ATOMICITY_INLINE int
__attribute_used__
test_and_set (volatile long int *p, long int newval)
{
  int result;
  __asm__ ("\n\
0:	lwarx	%0,0,%1	\n\
	cmpwi	%0,0	\n\
	bne-	1f	\n\
	stwcx.	%2,0,%1	\n\
	bne-	0b	\n\
1:	\n\
" : "=&r"(result) : "r"(p), "r"(newval) : "cr0", "memory");
  return result;
}

#endif /* atomicity.h */
