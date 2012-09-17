/* POSIX spinlock implementation.  ia64 version.
   Copyright (C) 2000, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jes Sorensen <jes@linuxcare.com>

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
#include <ia64intrin.h>

/* This implementation is inspired by the implementation used in the
   Linux kernel. */

int
__pthread_spin_lock (pthread_spinlock_t *lock)
{
  int *p = (int *) lock;

  while (__builtin_expect (__sync_val_compare_and_swap (p, 0, 1), 0))
    {
      /* Spin without using the atomic instruction.  */
      do
        __asm__ __volatile__ ("" : : : "memory");
      while (*p);
    }
  return 0;
}
weak_alias (__pthread_spin_lock, pthread_spin_lock)


int
__pthread_spin_trylock (pthread_spinlock_t *lock)
{
  return __sync_val_compare_and_swap ((int *) lock, 0, 1) == 0 ? 0 : EBUSY;
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
