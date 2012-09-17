/* Copyright (C) 2002, 2003, 2004, 2007 Free Software Foundation, Inc.
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

#include <errno.h>
#include "pthreadP.h"


int
attribute_protected
__pthread_cond_destroy (
     pthread_cond_t *cond)
{
  int pshared = (cond->__data.__mutex == (void *) ~0l)
		? LLL_SHARED : LLL_PRIVATE;

  /* Make sure we are alone.  */
  lll_lock (cond->__data.__lock, pshared);

  if (cond->__data.__total_seq > cond->__data.__wakeup_seq)
    {
      /* If there are still some waiters which have not been
	 woken up, this is an application bug.  */
      lll_unlock (cond->__data.__lock, pshared);
      return EBUSY;
    }

  /* Tell pthread_cond_*wait that this condvar is being destroyed.  */
  cond->__data.__total_seq = -1ULL;

  /* If there are waiters which have been already signalled or
     broadcasted, but still are using the pthread_cond_t structure,
     pthread_cond_destroy needs to wait for them.  */
  unsigned int nwaiters = cond->__data.__nwaiters;

  if (nwaiters >= (1 << COND_NWAITERS_SHIFT))
    {
      /* Wake everybody on the associated mutex in case there are
         threads that have been requeued to it.
         Without this, pthread_cond_destroy could block potentially
         for a long time or forever, as it would depend on other
         thread's using the mutex.
         When all threads waiting on the mutex are woken up, pthread_cond_wait
         only waits for threads to acquire and release the internal
         condvar lock.  */
      if (cond->__data.__mutex != NULL
	  && cond->__data.__mutex != (void *) ~0l)
	{
	  pthread_mutex_t *mut = (pthread_mutex_t *) cond->__data.__mutex;
	  lll_futex_wake (&mut->__data.__lock, INT_MAX,
			  PTHREAD_MUTEX_PSHARED (mut));
	}

      do
	{
	  lll_unlock (cond->__data.__lock, pshared);

	  lll_futex_wait (&cond->__data.__nwaiters, nwaiters, pshared);

	  lll_lock (cond->__data.__lock, pshared);

	  nwaiters = cond->__data.__nwaiters;
	}
      while (nwaiters >= (1 << COND_NWAITERS_SHIFT));
    }

  return 0;
}
weak_alias(__pthread_cond_destroy, pthread_cond_destroy)
