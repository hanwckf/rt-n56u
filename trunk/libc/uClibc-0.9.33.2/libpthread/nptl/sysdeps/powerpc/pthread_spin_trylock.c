/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

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

#include <errno.h>
#include "pthreadP.h"

int
pthread_spin_trylock (pthread_spinlock_t *lock)
{
  unsigned int old;
  int err = EBUSY;

  __asm__ ("1:	lwarx	%0,0,%2\n"
       "	cmpwi	0,%0,0\n"
       "	bne	2f\n"
       "	stwcx.	%3,0,%2\n"
       "	bne-	1b\n"
       "	li	%1,0\n"
       "	isync\n"
       "2:	"
       : "=&r" (old), "=&r" (err)
       : "r" (lock), "r" (1), "1" (err)
       : "cr0", "memory");

  return err;
}
