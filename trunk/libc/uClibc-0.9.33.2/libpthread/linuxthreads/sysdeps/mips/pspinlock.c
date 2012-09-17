/* POSIX spinlock implementation.  MIPS version.
   Copyright (C) 2000, 2002, 2003, 2004 Free Software Foundation, Inc.
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
#include <sgidefs.h>
#include <sys/tas.h>
#include "internals.h"

/* This implementation is similar to the one used in the Linux kernel.  */
int
__pthread_spin_lock (pthread_spinlock_t *lock)
{
  unsigned int tmp1, tmp2;

  __asm__ __volatile__
    ("\t\t\t# spin_lock\n"
     "1:\n\t"
     ".set	push\n\t"
#if _MIPS_SIM == _ABIO32
     ".set	mips2\n\t"
#endif
     "ll	%1,%3\n\t"
     "li	%2,1\n\t"
     "bnez	%1,1b\n\t"
     "sc	%2,%0\n\t"
     ".set	pop\n\t"
     "beqz	%2,1b"
     : "=m" (*lock), "=&r" (tmp1), "=&r" (tmp2)
     : "m" (*lock)
     : "memory");

  return 0;
}

weak_alias (__pthread_spin_lock, pthread_spin_lock)


int
__pthread_spin_trylock (pthread_spinlock_t *lock)
{
  /* To be done.  */
  return 0;
}
weak_alias (__pthread_spin_trylock, pthread_spin_trylock)


int
__pthread_spin_unlock (pthread_spinlock_t *lock)
{
  __asm__ __volatile__
    ("\t\t\t# spin_unlock\n\t"
     "sw	$0,%0"
     : "=m" (*lock)
     :
     : "memory");
  return 0;
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
