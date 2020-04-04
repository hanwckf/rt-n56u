/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* POSIX spinlock implementation.
   Copyright (C) 2000 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <pthread.h>
#include "internals.h"

int
__pthread_spin_lock (pthread_spinlock_t *lock)
{
	unsigned int		val ;
	unsigned int		temp ;
	unsigned int		offset = 0 ;

	__asm__ __volatile__ (
		"1:\n\t"
		"llw	%0,	[%1 + %2 << 0]\n\t"
		"bnez	%0, 1b\n\t"
		"movi	%3, #0x1\n\t"
		"scw	%3,	[%1 + %2 << 0]\n\t"
		"beqz	%3, 1b\n\t"
		: "=&r" (val)
		: "r" (lock), "r" (offset), "r" (temp)
		: "memory" ) ;
		
	return 0 ;
}
weak_alias (__pthread_spin_lock, pthread_spin_lock)


int
__pthread_spin_trylock (pthread_spinlock_t *lock)
{
	unsigned int		val ;
	unsigned int		temp ;
	unsigned int		offset = 0 ;

	__asm__ __volatile__ (
		"llw	%0,	[%1 + %2 << 0]\n\t"
		"bnez	%0, 1f\n\t"
		"movi	%3, #0x1\n\t"
		"scw	%3,	[%1 + %2 << 0]\n\t"
		"beqz	%3, 1f\n\t"
		"movi	%0, #0x0\n\t"
		"b	2f\n\t"
		"1:\n\t"
		"movi	%0, #16\n\t"
		"2:\n\t"
		: "=&r" (val)
		: "r" (lock), "r" (offset), "r" (temp)
		: "memory" ) ;
		
  return val;
}
weak_alias (__pthread_spin_trylock, pthread_spin_trylock)

int
__pthread_spin_unlock (pthread_spinlock_t *lock)
{
  return *lock = 0;
}
weak_alias (__pthread_spin_unlock, pthread_spin_unlock)


int
__pthread_spin_init (pthread_spinlock_t *lock, int pshared)
{
  /* We can ignore the `pshared' parameter.  Since we are busy-waiting
     all processes which can access the memory location `lock' points
     to can use the spinlock.  */
  return *lock = 0;
}
weak_alias (__pthread_spin_init, pthread_spin_init)


int
__pthread_spin_destroy (pthread_spinlock_t *lock)
{
  /* Nothing to do.  */
  return 0;
}
weak_alias (__pthread_spin_destroy, pthread_spin_destroy)
