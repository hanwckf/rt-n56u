/* Copyright (C) 2003, 2004, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <pthread.h>
#include <pthreadP.h>


/* Try to acquire write lock for RWLOCK or return after specfied time.	*/
int
pthread_rwlock_timedwrlock (
     pthread_rwlock_t *rwlock,
     const struct timespec *abstime)
{
  int result = 0;

  /* Make sure we are along.  */
  lll_lock (rwlock->__data.__lock, rwlock->__data.__shared);

  while (1)
    {
      int err;

      /* Get the rwlock if there is no writer and no reader.  */
      if (rwlock->__data.__writer == 0 && rwlock->__data.__nr_readers == 0)
	{
	  /* Mark self as writer.  */
	  rwlock->__data.__writer = THREAD_GETMEM (THREAD_SELF, tid);
	  break;
	}

      /* Make sure we are not holding the rwlock as a writer.  This is
	 a deadlock situation we recognize and report.  */
      if (__builtin_expect (rwlock->__data.__writer
			    == THREAD_GETMEM (THREAD_SELF, tid), 0))
	{
	  result = EDEADLK;
	  break;
	}

      /* Make sure the passed in timeout value is valid.  Ideally this
	 test would be executed once.  But since it must not be
	 performed if we would not block at all simply moving the test
	 to the front is no option.  Replicating all the code is
	 costly while this test is not.  */
      if (__builtin_expect (abstime->tv_nsec >= 1000000000
                            || abstime->tv_nsec < 0, 0))
	{
	  result = EINVAL;
	  break;
	}

      /* Get the current time.  So far we support only one clock.  */
      struct timeval tv;
      (void) gettimeofday (&tv, NULL);

      /* Convert the absolute timeout value to a relative timeout.  */
      struct timespec rt;
      rt.tv_sec = abstime->tv_sec - tv.tv_sec;
      rt.tv_nsec = abstime->tv_nsec - tv.tv_usec * 1000;
      if (rt.tv_nsec < 0)
	{
	  rt.tv_nsec += 1000000000;
	  --rt.tv_sec;
	}
      /* Did we already time out?  */
      if (rt.tv_sec < 0)
	{
	  result = ETIMEDOUT;
	  break;
	}

      /* Remember that we are a writer.  */
      if (++rwlock->__data.__nr_writers_queued == 0)
	{
	  /* Overflow on number of queued writers.  */
	  --rwlock->__data.__nr_writers_queued;
	  result = EAGAIN;
	  break;
	}

      int waitval = rwlock->__data.__writer_wakeup;

      /* Free the lock.  */
      lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);

      /* Wait for the writer or reader(s) to finish.  */
      err = lll_futex_timed_wait (&rwlock->__data.__writer_wakeup,
				  waitval, &rt, rwlock->__data.__shared);

      /* Get the lock.  */
      lll_lock (rwlock->__data.__lock, rwlock->__data.__shared);

      /* To start over again, remove the thread from the writer list.  */
      --rwlock->__data.__nr_writers_queued;

      /* Did the futex call time out?  */
      if (err == -ETIMEDOUT)
	{
	  result = ETIMEDOUT;
	  break;
	}
    }

  /* We are done, free the lock.  */
  lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);

  return result;
}
