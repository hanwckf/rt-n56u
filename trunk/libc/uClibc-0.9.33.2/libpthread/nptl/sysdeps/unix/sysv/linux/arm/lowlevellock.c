/* low level locking for pthread library.  Generic futex-using version.
   Copyright (C) 2003, 2005, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <sys/time.h>
#include <tls.h>

void
#ifndef IS_IN_libpthread
weak_function
#endif
__lll_lock_wait_private (int *futex)
{
  do
    {
      int oldval = atomic_compare_and_exchange_val_acq (futex, 2, 1);
      if (oldval != 0)
	lll_futex_wait (futex, 2, LLL_PRIVATE);
    }
  while (atomic_compare_and_exchange_bool_acq (futex, 2, 0) != 0);
}


/* These functions don't get included in libc.so  */
#ifdef IS_IN_libpthread
void
__lll_lock_wait (int *futex, int private)
{
  do
    {
      int oldval = atomic_compare_and_exchange_val_acq (futex, 2, 1);
      if (oldval != 0)
	lll_futex_wait (futex, 2, private);
    }
  while (atomic_compare_and_exchange_bool_acq (futex, 2, 0) != 0);
}


int
__lll_timedlock_wait (int *futex, const struct timespec *abstime, int private)
{
  struct timespec rt;

  /* Reject invalid timeouts.  */
  if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
    return EINVAL;

  /* Upgrade the lock.  */
  if (atomic_exchange_acq (futex, 2) == 0)
    return 0;

  do
    {
      struct timeval tv;

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
	return ETIMEDOUT;

      // XYZ: Lost the lock to check whether it was private.
      lll_futex_timed_wait (futex, 2, &rt, private);
    }
  while (atomic_compare_and_exchange_bool_acq (futex, 2, 0) != 0);

  return 0;
}


int
__lll_timedwait_tid (int *tidp, const struct timespec *abstime)
{
  int tid;

  if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
    return EINVAL;

  /* Repeat until thread terminated.  */
  while ((tid = *tidp) != 0)
    {
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
	return ETIMEDOUT;

      /* Wait until thread terminates.  */
      // XYZ: Lost the lock to check whether it was private.
      if (lll_futex_timed_wait (tidp, tid, &rt, LLL_SHARED) == -ETIMEDOUT)
	return ETIMEDOUT;
    }

  return 0;
}
#endif
