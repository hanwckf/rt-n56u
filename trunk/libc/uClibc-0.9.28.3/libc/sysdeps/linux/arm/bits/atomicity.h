/* Low-level functions for atomic operations.  ARM version.
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
#define _ATOMICITY_H    1

#include <inttypes.h>


static inline int
__attribute_used__
exchange_and_add (volatile uint32_t *mem, int val)
{
  int tmp1;
  int tmp2;
  int result;
  __asm__ ("\n"
#if defined(__thumb__)
	   "\t.align 0\n"
	   "\tbx pc\n"
	   "\tnop\n"
	   "\t.arm\n"
#endif
	   "0:\tldr\t%0,[%3]\n\t"
	   "add\t%1,%0,%4\n\t"
	   "swp\t%2,%1,[%3]\n\t"
	   "cmp\t%0,%2\n\t"
	   "swpne\t%1,%2,[%3]\n\t"
	   "bne\t0b"
#if defined(__thumb__)
	   "\torr %1, pc, #1\n"
	   "\tbx %1\n"
	   "\t.force_thumb"
#endif
	   : "=&r" (result), "=&r" (tmp1), "=&r" (tmp2)
	   : "r" (mem), "r"(val)
	   : "cc", "memory");
  return result;
}

static inline void
__attribute_used__
atomic_add (volatile uint32_t *mem, int val)
{
  int tmp1;
  int tmp2;
  int tmp3;
  __asm__ ("\n"
#if defined(__thumb__)
	   "\t.align 0\n"
	   "\tbx pc\n"
	   "\tnop\n"
	   "\t.arm\n"
#endif
	   "0:\tldr\t%0,[%3]\n\t"
	   "add\t%1,%0,%4\n\t"
	   "swp\t%2,%1,[%3]\n\t"
	   "cmp\t%0,%2\n\t"
	   "swpne\t%1,%2,[%3]\n\t"
	   "bne\t0b"
#if defined(__thumb__)
	   "\torr %1, pc, #1\n"
	   "\tbx %1\n"
	   "\t.force_thumb"
#endif
	   : "=&r" (tmp1), "=&r" (tmp2), "=&r" (tmp3)
	   : "r" (mem), "r"(val)
	   : "cc", "memory");
}

static inline int
__attribute_used__
compare_and_swap (volatile long int *p, long int oldval, long int newval)
{
  int result, tmp;
  __asm__ ("\n"
#if defined(__thumb__)
	   "\t.align 0\n"
	   "\tbx pc\n"
	   "\tnop\n"
	   "\t.arm\n"
#endif
	   "0:\tldr\t%1,[%2]\n\t"
	   "mov\t%0,#0\n\t"
	   "cmp\t%1,%4\n\t"
	   "bne\t1f\n\t"
	   "swp\t%0,%3,[%2]\n\t"
	   "cmp\t%1,%0\n\t"
	   "swpne\t%1,%0,[%2]\n\t"
	   "bne\t0b\n\t"
	   "mov\t%0,#1\n"
	   "1:"
#if defined(__thumb__)
	   "\torr %1, pc, #1\n"
	   "\tbx %1\n"
	   "\t.force_thumb"
#endif
	   : "=&r" (result), "=&r" (tmp)
	   : "r" (p), "r" (newval), "r" (oldval)
	   : "cc", "memory");
  return result;
}

#endif /* atomicity.h */
