/* POSIX spinlock implementation.  Alpha version.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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


/* This implementation is similar to the one used in the Linux kernel.
   But the kernel is byte instructions for the memory access.  This is
   faster but unusable here.  The problem is that only 128
   threads/processes could use the spinlock at the same time.  If (by
   a design error in the program) a thread/process would hold the
   spinlock for a time long enough to accumulate 128 waiting
   processes, the next one will find a positive value in the spinlock
   and assume it is unlocked.  We cannot accept that.  */

int
__pthread_spin_lock (pthread_spinlock_t *lock)
{
  unsigned int tmp;
  __asm__ __volatile__
    ("1:	ldl_l	%0,%1\n"
     "		blbs	%0,2f\n"
     "		or	%0,1,%0\n"
     "		stl_c	%0,%1\n"
     "		beq	%0,2f\n"
     "		mb\n"
     ".subsection 2\n"
     "2:	ldl	%0,%1\n"
     "		blbs	%0,2b\n"
     "		br	1b\n"
     ".previous"
     : "=r" (tmp), "=m" (lock)
     : "m" (lock));
  return 0;
}
weak_alias (__pthread_spin_lock, pthread_spin_lock)


int
__pthread_spin_trylock (pthread_spinlock_t *lock)
{
  unsigned long int oldval;
  unsigned long int temp;

  __asm__ __volatile__
    ("1:	ldl_l	%0,%1\n"
     "		and	%0,%3,%2\n"
     "		bne	%2,2f\n"
     "		xor	%0,%3,%0\n"
     "		stl_c	%0,%1\n"
     "		beq	%0,3f\n"
     "		mb\n"
     "2:\n"
     ".subsection 2\n"
     "3:	br	1b\n"
     ".previous"
     : "=&r" (temp), "=m" (*lock), "=&r" (oldval)
     : "Ir" (1UL), "m" (*lock));

  return oldval == 0 ? 0 : EBUSY;
}
weak_alias (__pthread_spin_trylock, pthread_spin_trylock)


int
__pthread_spin_unlock (pthread_spinlock_t *lock)
{
  __asm__ __volatile__ ("mb");
  return *lock = 0;
}
weak_alias (__pthread_spin_unlock, pthread_spin_unlock)


int
__pthread_spin_init (pthread_spinlock_t *lock, int pshared)
{
  /* We can ignore the `pshared' parameter.  Since we are busy-waiting
     all processes which can access the memory location `lock' points
     to can use the spinlock.  */
  *lock = 0;
  return 0;
}
weak_alias (__pthread_spin_init, pthread_spin_init)


int
__pthread_spin_destroy (pthread_spinlock_t *lock)
{
  /* Nothing to do.  */
  return 0;
}
weak_alias (__pthread_spin_destroy, pthread_spin_destroy)
