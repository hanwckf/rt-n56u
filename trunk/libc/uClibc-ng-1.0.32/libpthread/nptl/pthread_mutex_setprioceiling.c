/* Set current priority ceiling of pthread_mutex_t.
   Copyright (C) 2006, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2006.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdbool.h>
#include <errno.h>
#include <pthreadP.h>


int
pthread_mutex_setprioceiling (mutex, prioceiling, old_ceiling)
     pthread_mutex_t *mutex;
     int prioceiling;
     int *old_ceiling;
{
  /* The low bits of __kind aren't ever changed after pthread_mutex_init,
     so we don't need a lock yet.  */
  if ((mutex->__data.__kind & PTHREAD_MUTEX_PRIO_PROTECT_NP) == 0)
    return EINVAL;

  if (__sched_fifo_min_prio == -1)
    __init_sched_fifo_prio ();

  if (__builtin_expect (prioceiling < __sched_fifo_min_prio, 0)
      || __builtin_expect (prioceiling > __sched_fifo_max_prio, 0)
      || __builtin_expect ((prioceiling
			    & (PTHREAD_MUTEXATTR_PRIO_CEILING_MASK
			       >> PTHREAD_MUTEXATTR_PRIO_CEILING_SHIFT))
			   != prioceiling, 0))
    return EINVAL;

  /* Check whether we already hold the mutex.  */
  bool locked = false;
  int kind = PTHREAD_MUTEX_TYPE (mutex);
  if (mutex->__data.__owner == THREAD_GETMEM (THREAD_SELF, tid))
    {
      if (kind == PTHREAD_MUTEX_PP_ERRORCHECK_NP)
	return EDEADLK;

      if (kind == PTHREAD_MUTEX_PP_RECURSIVE_NP)
	locked = true;
    }

  int oldval = mutex->__data.__lock;
  if (! locked)
    do
      {
	/* Need to lock the mutex, but without obeying the priority
	   protect protocol.  */
	int ceilval = (oldval & PTHREAD_MUTEX_PRIO_CEILING_MASK);

	oldval = atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
						      ceilval | 1, ceilval);
	if (oldval == ceilval)
	  break;

	do
	  {
	    oldval
	      = atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
						     ceilval | 2,
						     ceilval | 1);

	    if ((oldval & PTHREAD_MUTEX_PRIO_CEILING_MASK) != ceilval)
	      break;

	    if (oldval != ceilval)
	      lll_futex_wait (&mutex->__data.__lock, ceilval | 2,
			      PTHREAD_MUTEX_PSHARED (mutex));
	  }
	while (atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
						    ceilval | 2, ceilval)
	       != ceilval);

	if ((oldval & PTHREAD_MUTEX_PRIO_CEILING_MASK) != ceilval)
	  continue;
      }
    while (0);

  int oldprio = (oldval & PTHREAD_MUTEX_PRIO_CEILING_MASK)
		>> PTHREAD_MUTEX_PRIO_CEILING_SHIFT;
  if (locked)
    {
      int ret = __pthread_tpp_change_priority (oldprio, prioceiling);
      if (ret)
	return ret;
    }

  if (old_ceiling != NULL)
    *old_ceiling = oldprio;

  int newlock = 0;
  if (locked)
    newlock = (mutex->__data.__lock & ~PTHREAD_MUTEX_PRIO_CEILING_MASK);
  mutex->__data.__lock = newlock
			 | (prioceiling << PTHREAD_MUTEX_PRIO_CEILING_SHIFT);
  atomic_full_barrier ();

  lll_futex_wake (&mutex->__data.__lock, INT_MAX,
		  PTHREAD_MUTEX_PSHARED (mutex));

  return 0;
}
