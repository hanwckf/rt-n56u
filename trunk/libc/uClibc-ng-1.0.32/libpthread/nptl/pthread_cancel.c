/* Copyright (C) 2002, 2003, 2004, 2009 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <signal.h>
#include "pthreadP.h"
#include "atomic.h"
#include <sysdep.h>
#include <unistd.h>
#include <bits/kernel-features.h>


int
pthread_cancel (
     pthread_t th)
{
  volatile struct pthread *pd = (volatile struct pthread *) th;

  /* Make sure the descriptor is valid.  */
  if (INVALID_TD_P (pd))
    /* Not a valid thread handle.  */
    return ESRCH;

#ifdef SHARED
  pthread_cancel_init ();
#endif
  int result = 0;
  int oldval;
  int newval;
  do
    {
    again:
      oldval = pd->cancelhandling;
      newval = oldval | CANCELING_BITMASK | CANCELED_BITMASK;

      /* Avoid doing unnecessary work.  The atomic operation can
	 potentially be expensive if the bug has to be locked and
	 remote cache lines have to be invalidated.  */
      if (oldval == newval)
	break;

      /* If the cancellation is handled asynchronously just send a
	 signal.  We avoid this if possible since it's more
	 expensive.  */
      if (CANCEL_ENABLED_AND_CANCELED_AND_ASYNCHRONOUS (newval))
	{
	  /* Mark the cancellation as "in progress".  */
	  if (atomic_compare_and_exchange_bool_acq (&pd->cancelhandling,
						    oldval | CANCELING_BITMASK,
						    oldval))
	    goto again;

	  /* The cancellation handler will take care of marking the
	     thread as canceled.  */
	  INTERNAL_SYSCALL_DECL (err);

	  /* One comment: The PID field in the TCB can temporarily be
	     changed (in fork).  But this must not affect this code
	     here.  Since this function would have to be called while
	     the thread is executing fork, it would have to happen in
	     a signal handler.  But this is no allowed, pthread_cancel
	     is not guaranteed to be async-safe.  */
	  pid_t pid = getpid ();
	  int val;
	  val = INTERNAL_SYSCALL (tgkill, err, 3, pid, pd->tid, SIGCANCEL);

	  if (INTERNAL_SYSCALL_ERROR_P (val, err))
	    result = INTERNAL_SYSCALL_ERRNO (val, err);

	  break;
	}
    }
  /* Mark the thread as canceled.  This has to be done
     atomically since other bits could be modified as well.  */
  while (atomic_compare_and_exchange_bool_acq (&pd->cancelhandling, newval,
					       oldval));

  return result;
}

PTHREAD_STATIC_FN_REQUIRE (pthread_create)
