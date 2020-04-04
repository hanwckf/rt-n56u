/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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
#include "pthreadP.h"
#include <atomic.h>


int
attribute_protected
__pthread_setcancelstate (
     int state,
     int *oldstate)
{
  volatile struct pthread *self;

  if (state < PTHREAD_CANCEL_ENABLE || state > PTHREAD_CANCEL_DISABLE)
    return EINVAL;

  self = THREAD_SELF;

  int oldval = THREAD_GETMEM (self, cancelhandling);
  while (1)
    {
      int newval = (state == PTHREAD_CANCEL_DISABLE
		    ? oldval | CANCELSTATE_BITMASK
		    : oldval & ~CANCELSTATE_BITMASK);

      /* Store the old value.  */
      if (oldstate != NULL)
	*oldstate = ((oldval & CANCELSTATE_BITMASK)
		     ? PTHREAD_CANCEL_DISABLE : PTHREAD_CANCEL_ENABLE);

      /* Avoid doing unnecessary work.  The atomic operation can
	 potentially be expensive if the memory has to be locked and
	 remote cache lines have to be invalidated.  */
      if (oldval == newval)
	break;

      /* Update the cancel handling word.  This has to be done
	 atomically since other bits could be modified as well.  */
      int curval = THREAD_ATOMIC_CMPXCHG_VAL (self, cancelhandling, newval,
					      oldval);
      if (__builtin_expect (curval == oldval, 1))
	{
	  if (CANCEL_ENABLED_AND_CANCELED_AND_ASYNCHRONOUS (newval))
	    __do_cancel ();

	  break;
	}

      /* Prepare for the next round.  */
      oldval = curval;
    }

  return 0;
}
strong_alias (__pthread_setcancelstate, pthread_setcancelstate)
