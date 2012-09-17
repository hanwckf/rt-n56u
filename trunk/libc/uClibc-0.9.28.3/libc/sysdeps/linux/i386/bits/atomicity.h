/* Low-level functions for atomic operations.  ix86 version, x >= 4.
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
__attribute_used__
exchange_and_add (volatile uint32_t *mem, uint32_t val)
{
  register uint32_t result;
  __asm__ __volatile__ ("lock; xaddl %0,%1"
			: "=r" (result), "=m" (*mem) : "0" (val), "1" (*mem));
  return result;
}

static inline void
__attribute_used__
atomic_add (volatile uint32_t *mem, int val)
{
  __asm__ __volatile__ ("lock; addl %1,%0"
			: "=m" (*mem) : "ir" (val), "0" (*mem));
}

static inline char
__attribute_used__
compare_and_swap (volatile long int *p, long int oldval, long int newval)
{
  char ret;
  long int readval;

  __asm__ __volatile__ ("lock; cmpxchgl %3, %1; sete %0"
                        : "=q" (ret), "=m" (*p), "=a" (readval)
                        : "r" (newval), "1" (*p), "a" (oldval));
  return ret;
}

#endif /* atomicity.h */
