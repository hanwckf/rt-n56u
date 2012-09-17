/* Low-level functions for atomic operations.  H8/300 version.
   Copyright (C) 1997, 2000, 2001 Free Software Foundation, Inc.
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


static inline uint32_t
__attribute__ ((unused))
exchange_and_add (volatile uint32_t *mem, uint32_t val)
{
  uint32_t result;
  __asm__ __volatile__ ("stc ccr,@-sp\n\t"
			"orc #0x80,ccr\n\t"
			"mov.l %1,er1\n\t"
			"mov.l %0,%1\n\t"
			"add.l er1,%0\n\t"
			"ldc @sp+,ccr"
			: "=r" (result), "=m" (*mem) 
			: "0" (val), "1" (*mem)
			: "er1");
  return result;
}

static inline void
__attribute__ ((unused))
atomic_add (volatile uint32_t *mem, int val)
{
  __asm__ __volatile__ ("stc ccr,@-sp\n\t"
			"orc #0x80,ccr\n\t"
			"mov.l %0,er0\n\t"
			"add %1,er0\n\t"
			"mov.l er0,%0\n\t"
			"ldc @sp+,ccr"
			: "=m" (*mem) 
			: "r" (val), "0" (*mem)
			: "er0");
}

static inline int
__attribute__ ((unused))
compare_and_swap (volatile long int *p, long int oldval, long int newval)
{
  int ret = 0;

  __asm__ __volatile__ ("stc ccr,@-sp\n\t"
			"orc #0x80,ccr\n\t"
			"mov.l %1,er0\n\t"
			"cmp.l %2,er0\n\t"
			"bne 1f\n\t"
			"mov.l %3,%1\n\t"
			"inc.l #1,%0\n"
			"1:\n\t"
			"ldc @sp+,ccr"
			: "=r"(ret),"=m"(*p)
			: "r"(oldval),"r"(newval),"0"(ret),"1"(*p)
			: "er0");
  return ret;
}

#endif /* atomicity.h */
