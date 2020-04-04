/* Copyright (C) 2003, 2004, 2006, 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <endian.h>
#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <pthread.h>
#include <pthreadP.h>

#include <bits/kernel-features.h>


int
attribute_protected
__pthread_cond_broadcast (
     pthread_cond_t *cond)
{
  int pshared = (cond->__data.__mutex == (void *) ~0l)
		? LLL_SHARED : LLL_PRIVATE;
  /* Make sure we are alone.  */
  lll_lock (cond->__data.__lock, pshared);

  /* Are there any waiters to be woken?  */
  if (cond->__data.__total_seq > cond->__data.__wakeup_seq)
    {
      /* Yes.  Mark them all as woken.  */
      cond->__data.__wakeup_seq = cond->__data.__total_seq;
      cond->__data.__woken_seq = cond->__data.__total_seq;
      cond->__data.__futex = (unsigned int) cond->__data.__total_seq * 2;
      int futex_val = cond->__data.__futex;
      /* Signal that a broadcast happened.  */
      ++cond->__data.__broadcast_seq;

      /* We are done.  */
      lll_unlock (cond->__data.__lock, pshared);

      /* Do not use requeue for pshared condvars.  */
      if (cond->__data.__mutex == (void *) ~0l)
	goto wake_all;

      /* Wake everybody.  */
      pthread_mutex_t *mut = (pthread_mutex_t *) cond->__data.__mutex;

      /* XXX: Kernel so far doesn't support requeue to PI futex.  */
      /* XXX: Kernel so far can only requeue to the same type of futex,
	 in this case private (we don't requeue for pshared condvars).  */
      if (__builtin_expect (mut->__data.__kind
			    & (PTHREAD_MUTEX_PRIO_INHERIT_NP
			       | PTHREAD_MUTEX_PSHARED_BIT), 0))
	goto wake_all;

      /* lll_futex_requeue returns 0 for success and non-zero
	 for errors.  */
      if (__builtin_expect (lll_futex_requeue (&cond->__data.__futex, 1,
					       INT_MAX, &mut->__data.__lock,
					       futex_val, LLL_PRIVATE), 0))
	{
	  /* The requeue functionality is not available.  */
	wake_all:
	  lll_futex_wake (&cond->__data.__futex, INT_MAX, pshared);
	}

      /* That's all.  */
      return 0;
    }

  /* We are done.  */
  lll_unlock (cond->__data.__lock, pshared);

  return 0;
}

weak_alias(__pthread_cond_broadcast, pthread_cond_broadcast)
