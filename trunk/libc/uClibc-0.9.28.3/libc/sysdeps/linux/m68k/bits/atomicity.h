/* Low-level functions for atomic operations.  m680x0 version, x >= 2.
   Copyright (C) 1997 Free Software Foundation, Inc.
   Contributed by Andreas Schwab <schwab@issan.informatik.uni-dortmund.de>.
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


static inline int
__attribute_used__
exchange_and_add (volatile uint32_t *mem, int val)
{
  register int result = *mem;
  register int temp;
  __asm__ __volatile__ ("1: move%.l %0,%1;"
			"   add%.l %2,%1;"
			"   cas%.l %0,%1,%3;"
			"   jbne 1b"
			: "=d" (result), "=&d" (temp)
			: "d" (val), "m" (*mem), "0" (result) : "memory");
  return result;
}

static inline void
__attribute_used__
atomic_add (volatile uint32_t *mem, int val)
{
  /* XXX Use cas here as well?  */
  __asm__ __volatile__ ("add%.l %0,%1"
			: : "id" (val), "m" (*mem) : "memory");
}

static inline int
__attribute_used__
compare_and_swap (volatile long int *p, long int oldval, long int newval)
{
  char ret;
  long int readval;

  __asm__ __volatile__ ("cas%.l %2,%3,%1; seq %0"
                        : "=dm" (ret), "=m" (*p), "=d" (readval)
                        : "d" (newval), "m" (*p), "2" (oldval));
  return ret;
}

#endif /* atomicity.h */
