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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include "pthreadP.h"
#include <atomic.h>


int
attribute_protected
__pthread_setcanceltype (
     int type,
     int *oldtype)
{
  volatile struct pthread *self;

  if (type < PTHREAD_CANCEL_DEFERRED || type > PTHREAD_CANCEL_ASYNCHRONOUS)
    return EINVAL;

  self = THREAD_SELF;

  int oldval = THREAD_GETMEM (self, cancelhandling);
  while (1)
    {
      int newval = (type == PTHREAD_CANCEL_ASYNCHRONOUS
		    ? oldval | CANCELTYPE_BITMASK
		    : oldval & ~CANCELTYPE_BITMASK);

      /* Store the old value.  */
      if (oldtype != NULL)
	*oldtype = ((oldval & CANCELTYPE_BITMASK)
		    ? PTHREAD_CANCEL_ASYNCHRONOUS : PTHREAD_CANCEL_DEFERRED);

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
	    {
	      THREAD_SETMEM (self, result, PTHREAD_CANCELED);
	      __do_cancel ();
	    }

	  break;
	}

      /* Prepare for the next round.  */
      oldval = curval;
    }

  return 0;
}
strong_alias (__pthread_setcanceltype, pthread_setcanceltype)
