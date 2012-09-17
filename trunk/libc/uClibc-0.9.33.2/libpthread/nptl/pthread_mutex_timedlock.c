/* Copyright (C) 2002-2007, 2008 Free Software Foundation, Inc.
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

#include <assert.h>
#include <errno.h>
#include <time.h>
#include "pthreadP.h"
#include <lowlevellock.h>
#include <not-cancel.h>

/* We need to build this function with optimization to avoid
 * lll_timedlock erroring out with
 * error: can't find a register in class ‘GENERAL_REGS’ while reloading ‘asm’
 */
int
attribute_optimize("Os")
pthread_mutex_timedlock (
     pthread_mutex_t *mutex,
     const struct timespec *abstime)
{
  int oldval;
  pid_t id = THREAD_GETMEM (THREAD_SELF, tid);
  int result = 0;

  /* We must not check ABSTIME here.  If the thread does not block
     abstime must not be checked for a valid value.  */

  switch (__builtin_expect (PTHREAD_MUTEX_TYPE (mutex),
			    PTHREAD_MUTEX_TIMED_NP))
    {
      /* Recursive mutex.  */
    case PTHREAD_MUTEX_RECURSIVE_NP:
      /* Check whether we already hold the mutex.  */
      if (mutex->__data.__owner == id)
	{
	  /* Just bump the counter.  */
	  if (__builtin_expect (mutex->__data.__count + 1 == 0, 0))
	    /* Overflow of the counter.  */
	    return EAGAIN;

	  ++mutex->__data.__count;

	  goto out;
	}

      /* We have to get the mutex.  */
      result = lll_timedlock (mutex->__data.__lock, abstime,
			      PTHREAD_MUTEX_PSHARED (mutex));

      if (result != 0)
	goto out;

      /* Only locked once so far.  */
      mutex->__data.__count = 1;
      break;

      /* Error checking mutex.  */
    case PTHREAD_MUTEX_ERRORCHECK_NP:
      /* Check whether we already hold the mutex.  */
      if (__builtin_expect (mutex->__data.__owner == id, 0))
	return EDEADLK;

      /* FALLTHROUGH */

    case PTHREAD_MUTEX_TIMED_NP:
    simple:
      /* Normal mutex.  */
      result = lll_timedlock (mutex->__data.__lock, abstime,
			      PTHREAD_MUTEX_PSHARED (mutex));
      break;

    case PTHREAD_MUTEX_ADAPTIVE_NP:
      if (! __is_smp)
	goto simple;

      if (lll_trylock (mutex->__data.__lock) != 0)
	{
	  int cnt = 0;
	  int max_cnt = MIN (MAX_ADAPTIVE_COUNT,
			     mutex->__data.__spins * 2 + 10);
	  do
	    {
	      if (cnt++ >= max_cnt)
		{
		  result = lll_timedlock (mutex->__data.__lock, abstime,
					  PTHREAD_MUTEX_PSHARED (mutex));
		  break;
		}

#ifdef BUSY_WAIT_NOP
	      BUSY_WAIT_NOP;
#endif
	    }
	  while (lll_trylock (mutex->__data.__lock) != 0);

	  mutex->__data.__spins += (cnt - mutex->__data.__spins) / 8;
	}
      break;

    case PTHREAD_MUTEX_ROBUST_RECURSIVE_NP:
    case PTHREAD_MUTEX_ROBUST_ERRORCHECK_NP:
    case PTHREAD_MUTEX_ROBUST_NORMAL_NP:
    case PTHREAD_MUTEX_ROBUST_ADAPTIVE_NP:
      THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending,
		     &mutex->__data.__list.__next);

      oldval = mutex->__data.__lock;
      do
	{
	again:
	  if ((oldval & FUTEX_OWNER_DIED) != 0)
	    {
	      /* The previous owner died.  Try locking the mutex.  */
	      int newval = id | (oldval & FUTEX_WAITERS);

	      newval
		= atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
						       newval, oldval);
	      if (newval != oldval)
		{
		  oldval = newval;
		  goto again;
		}

	      /* We got the mutex.  */
	      mutex->__data.__count = 1;
	      /* But it is inconsistent unless marked otherwise.  */
	      mutex->__data.__owner = PTHREAD_MUTEX_INCONSISTENT;

	      ENQUEUE_MUTEX (mutex);
	      THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);

	      /* Note that we deliberately exit here.  If we fall
		 through to the end of the function __nusers would be
		 incremented which is not correct because the old
		 owner has to be discounted.  */
	      return EOWNERDEAD;
	    }

	  /* Check whether we already hold the mutex.  */
	  if (__builtin_expect ((oldval & FUTEX_TID_MASK) == id, 0))
	    {
	      int kind = PTHREAD_MUTEX_TYPE (mutex);
	      if (kind == PTHREAD_MUTEX_ROBUST_ERRORCHECK_NP)
		{
		  THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending,
				 NULL);
		  return EDEADLK;
		}

	      if (kind == PTHREAD_MUTEX_ROBUST_RECURSIVE_NP)
		{
		  THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending,
				 NULL);

		  /* Just bump the counter.  */
		  if (__builtin_expect (mutex->__data.__count + 1 == 0, 0))
		    /* Overflow of the counter.  */
		    return EAGAIN;

		  ++mutex->__data.__count;

		  return 0;
		}
	    }

	  result = lll_robust_timedlock (mutex->__data.__lock, abstime, id,
					 PTHREAD_ROBUST_MUTEX_PSHARED (mutex));

	  if (__builtin_expect (mutex->__data.__owner
				== PTHREAD_MUTEX_NOTRECOVERABLE, 0))
	    {
	      /* This mutex is now not recoverable.  */
	      mutex->__data.__count = 0;
	      lll_unlock (mutex->__data.__lock,
			  PTHREAD_ROBUST_MUTEX_PSHARED (mutex));
	      THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
	      return ENOTRECOVERABLE;
	    }

	  if (result == ETIMEDOUT || result == EINVAL)
	    goto out;

	  oldval = result;
	}
      while ((oldval & FUTEX_OWNER_DIED) != 0);

      mutex->__data.__count = 1;
      ENQUEUE_MUTEX (mutex);
      THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
      break;

    case PTHREAD_MUTEX_PI_RECURSIVE_NP:
    case PTHREAD_MUTEX_PI_ERRORCHECK_NP:
    case PTHREAD_MUTEX_PI_NORMAL_NP:
    case PTHREAD_MUTEX_PI_ADAPTIVE_NP:
    case PTHREAD_MUTEX_PI_ROBUST_RECURSIVE_NP:
    case PTHREAD_MUTEX_PI_ROBUST_ERRORCHECK_NP:
    case PTHREAD_MUTEX_PI_ROBUST_NORMAL_NP:
    case PTHREAD_MUTEX_PI_ROBUST_ADAPTIVE_NP:
      {
	int kind = mutex->__data.__kind & PTHREAD_MUTEX_KIND_MASK_NP;
	int robust = mutex->__data.__kind & PTHREAD_MUTEX_ROBUST_NORMAL_NP;

	if (robust)
	  /* Note: robust PI futexes are signaled by setting bit 0.  */
	  THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending,
			 (void *) (((uintptr_t) &mutex->__data.__list.__next)
				   | 1));

	oldval = mutex->__data.__lock;

	/* Check whether we already hold the mutex.  */
	if (__builtin_expect ((oldval & FUTEX_TID_MASK) == id, 0))
	  {
	    if (kind == PTHREAD_MUTEX_ERRORCHECK_NP)
	      {
		THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
		return EDEADLK;
	      }

	    if (kind == PTHREAD_MUTEX_RECURSIVE_NP)
	      {
		THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);

		/* Just bump the counter.  */
		if (__builtin_expect (mutex->__data.__count + 1 == 0, 0))
		  /* Overflow of the counter.  */
		  return EAGAIN;

		++mutex->__data.__count;

		return 0;
	      }
	  }

	oldval = atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
						      id, 0);

	if (oldval != 0)
	  {
	    /* The mutex is locked.  The kernel will now take care of
	       everything.  The timeout value must be a relative value.
	       Convert it.  */
	    int private = (robust
			   ? PTHREAD_ROBUST_MUTEX_PSHARED (mutex)
			   : PTHREAD_MUTEX_PSHARED (mutex));
	    INTERNAL_SYSCALL_DECL (__err);

	    int e = INTERNAL_SYSCALL (futex, __err, 4, &mutex->__data.__lock,
				      __lll_private_flag (FUTEX_LOCK_PI,
							  private), 1,
				      abstime);
	    if (INTERNAL_SYSCALL_ERROR_P (e, __err))
	      {
		if (INTERNAL_SYSCALL_ERRNO (e, __err) == ETIMEDOUT)
		  return ETIMEDOUT;

		if (INTERNAL_SYSCALL_ERRNO (e, __err) == ESRCH
		    || INTERNAL_SYSCALL_ERRNO (e, __err) == EDEADLK)
		  {
		    assert (INTERNAL_SYSCALL_ERRNO (e, __err) != EDEADLK
			    || (kind != PTHREAD_MUTEX_ERRORCHECK_NP
				&& kind != PTHREAD_MUTEX_RECURSIVE_NP));
		    /* ESRCH can happen only for non-robust PI mutexes where
		       the owner of the lock died.  */
		    assert (INTERNAL_SYSCALL_ERRNO (e, __err) != ESRCH
			    || !robust);

		    /* Delay the thread until the timeout is reached.
		       Then return ETIMEDOUT.  */
		    struct timespec reltime;
		    struct timespec now;

		    INTERNAL_SYSCALL (clock_gettime, __err, 2, CLOCK_REALTIME,
				      &now);
		    reltime.tv_sec = abstime->tv_sec - now.tv_sec;
		    reltime.tv_nsec = abstime->tv_nsec - now.tv_nsec;
		    if (reltime.tv_nsec < 0)
		      {
			reltime.tv_nsec += 1000000000;
			--reltime.tv_sec;
		      }
		    if (reltime.tv_sec >= 0)
		      while (nanosleep_not_cancel (&reltime, &reltime) != 0)
			continue;

		    return ETIMEDOUT;
		  }

		return INTERNAL_SYSCALL_ERRNO (e, __err);
	      }

	    oldval = mutex->__data.__lock;

	    assert (robust || (oldval & FUTEX_OWNER_DIED) == 0);
	  }

	if (__builtin_expect (oldval & FUTEX_OWNER_DIED, 0))
	  {
	    atomic_and (&mutex->__data.__lock, ~FUTEX_OWNER_DIED);

	    /* We got the mutex.  */
	    mutex->__data.__count = 1;
	    /* But it is inconsistent unless marked otherwise.  */
	    mutex->__data.__owner = PTHREAD_MUTEX_INCONSISTENT;

	    ENQUEUE_MUTEX_PI (mutex);
	    THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);

	    /* Note that we deliberately exit here.  If we fall
	       through to the end of the function __nusers would be
	       incremented which is not correct because the old owner
	       has to be discounted.  */
	    return EOWNERDEAD;
	  }

	if (robust
	    && __builtin_expect (mutex->__data.__owner
				 == PTHREAD_MUTEX_NOTRECOVERABLE, 0))
	  {
	    /* This mutex is now not recoverable.  */
	    mutex->__data.__count = 0;

	    INTERNAL_SYSCALL_DECL (__err);
	    INTERNAL_SYSCALL (futex, __err, 4, &mutex->__data.__lock,
			      __lll_private_flag (FUTEX_UNLOCK_PI,
						  PTHREAD_ROBUST_MUTEX_PSHARED (mutex)),
			      0, 0);

	    THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
	    return ENOTRECOVERABLE;
	  }

	mutex->__data.__count = 1;
	if (robust)
	  {
	    ENQUEUE_MUTEX_PI (mutex);
	    THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
	  }
	}
      break;

    case PTHREAD_MUTEX_PP_RECURSIVE_NP:
    case PTHREAD_MUTEX_PP_ERRORCHECK_NP:
    case PTHREAD_MUTEX_PP_NORMAL_NP:
    case PTHREAD_MUTEX_PP_ADAPTIVE_NP:
      {
	int kind = mutex->__data.__kind & PTHREAD_MUTEX_KIND_MASK_NP;

	oldval = mutex->__data.__lock;

	/* Check whether we already hold the mutex.  */
	if (mutex->__data.__owner == id)
	  {
	    if (kind == PTHREAD_MUTEX_ERRORCHECK_NP)
	      return EDEADLK;

	    if (kind == PTHREAD_MUTEX_RECURSIVE_NP)
	      {
		/* Just bump the counter.  */
		if (__builtin_expect (mutex->__data.__count + 1 == 0, 0))
		  /* Overflow of the counter.  */
		  return EAGAIN;

		++mutex->__data.__count;

		return 0;
	      }
	  }

	int oldprio = -1, ceilval;
	do
	  {
	    int ceiling = (oldval & PTHREAD_MUTEX_PRIO_CEILING_MASK)
			  >> PTHREAD_MUTEX_PRIO_CEILING_SHIFT;

	    if (__pthread_current_priority () > ceiling)
	      {
		result = EINVAL;
	      failpp:
		if (oldprio != -1)
		  __pthread_tpp_change_priority (oldprio, -1);
		return result;
	      }

	    result = __pthread_tpp_change_priority (oldprio, ceiling);
	    if (result)
	      return result;

	    ceilval = ceiling << PTHREAD_MUTEX_PRIO_CEILING_SHIFT;
	    oldprio = ceiling;

	    oldval
	      = atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
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
		  {
		    /* Reject invalid timeouts.  */
		    if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
		      {
			result = EINVAL;
			goto failpp;
		      }

		    struct timeval tv;
		    struct timespec rt;

		    /* Get the current time.  */
		    (void) gettimeofday (&tv, NULL);

		    /* Compute relative timeout.  */
		    rt.tv_sec = abstime->tv_sec - tv.tv_sec;
		    rt.tv_nsec = abstime->tv_nsec - tv.tv_usec * 1000;
		    if (rt.tv_nsec < 0)
		      {
			rt.tv_nsec += 1000000000;
			--rt.tv_sec;
		      }

		    /* Already timed out?  */
		    if (rt.tv_sec < 0)
		      {
			result = ETIMEDOUT;
			goto failpp;
		      }

		    lll_futex_timed_wait (&mutex->__data.__lock,
					  ceilval | 2, &rt,
					  PTHREAD_MUTEX_PSHARED (mutex));
		  }
	      }
	    while (atomic_compare_and_exchange_val_acq (&mutex->__data.__lock,
							ceilval | 2, ceilval)
		   != ceilval);
	  }
	while ((oldval & PTHREAD_MUTEX_PRIO_CEILING_MASK) != ceilval);

	assert (mutex->__data.__owner == 0);
	mutex->__data.__count = 1;
      }
      break;

    default:
      /* Correct code cannot set any other type.  */
      return EINVAL;
    }

  if (result == 0)
    {
      /* Record the ownership.  */
      mutex->__data.__owner = id;
      ++mutex->__data.__nusers;
    }

 out:
  return result;
}
