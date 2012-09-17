/* Low level locking macros used in NPTL implementation.  Stub version.
   Copyright (C) 2002, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <atomic.h>


/* Mutex lock counter:
   bit 31 clear means unlocked;
   bit 31 set means locked.

   All code that looks at bit 31 first increases the 'number of
   interested threads' usage counter, which is in bits 0-30.

   All negative mutex values indicate that the mutex is still locked.  */


static inline void
__generic_mutex_lock (int *mutex)
{
  unsigned int v;

  /* Bit 31 was clear, we got the mutex.  (this is the fastpath).  */
  if (atomic_bit_test_set (mutex, 31) == 0)
    return;

  atomic_increment (mutex);

  while (1)
    {
      if (atomic_bit_test_set (mutex, 31) == 0)
	{
	  atomic_decrement (mutex);
	  return;
	}

      /* We have to wait now. First make sure the futex value we are
	 monitoring is truly negative (i.e. locked). */
      v = *mutex;
      if (v >= 0)
	continue;

      lll_futex_wait (mutex, v,
		      // XYZ check mutex flag
		      LLL_SHARED);
    }
}


static inline void
__generic_mutex_unlock (int *mutex)
{
  /* Adding 0x80000000 to the counter results in 0 if and only if
     there are not other interested threads - we can return (this is
     the fastpath).  */
  if (atomic_add_zero (mutex, 0x80000000))
    return;

  /* There are other threads waiting for this mutex, wake one of them
     up.  */
  lll_futex_wake (mutex, 1,
		  // XYZ check mutex flag
		  LLL_SHARED);
}


#define lll_mutex_lock(futex) __generic_mutex_lock (&(futex))
#define lll_mutex_unlock(futex) __generic_mutex_unlock (&(futex))
