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

#include <stdlib.h>
#include "pthreadP.h"


void
__cleanup_fct_attribute
__pthread_register_cancel_defer (__pthread_unwind_buf_t *buf)
{
  struct pthread_unwind_buf *ibuf = (struct pthread_unwind_buf *) buf;
  struct pthread *self = THREAD_SELF;

  /* Store old info.  */
  ibuf->priv.data.prev = THREAD_GETMEM (self, cleanup_jmp_buf);
  ibuf->priv.data.cleanup = THREAD_GETMEM (self, cleanup);

  int cancelhandling = THREAD_GETMEM (self, cancelhandling);

  /* Disable asynchronous cancellation for now.  */
  if (__builtin_expect (cancelhandling & CANCELTYPE_BITMASK, 0))
    while (1)
      {
	int curval = THREAD_ATOMIC_CMPXCHG_VAL (self, cancelhandling,
						cancelhandling
						& ~CANCELTYPE_BITMASK,
						cancelhandling);
	if (__builtin_expect (curval == cancelhandling, 1))
	  /* Successfully replaced the value.  */
	  break;

	/* Prepare for the next round.  */
	cancelhandling = curval;
      }

  ibuf->priv.data.canceltype = (cancelhandling & CANCELTYPE_BITMASK
				? PTHREAD_CANCEL_ASYNCHRONOUS
				: PTHREAD_CANCEL_DEFERRED);

  /* Store the new cleanup handler info.  */
  THREAD_SETMEM (self, cleanup_jmp_buf, (struct pthread_unwind_buf *) buf);
}


void
__cleanup_fct_attribute
__pthread_unregister_cancel_restore (__pthread_unwind_buf_t *buf)
{
  struct pthread *self = THREAD_SELF;
  struct pthread_unwind_buf *ibuf = (struct pthread_unwind_buf *) buf;

  THREAD_SETMEM (self, cleanup_jmp_buf, ibuf->priv.data.prev);

  int cancelhandling;
  if (ibuf->priv.data.canceltype != PTHREAD_CANCEL_DEFERRED
      && ((cancelhandling = THREAD_GETMEM (self, cancelhandling))
	  & CANCELTYPE_BITMASK) == 0)
    {
      while (1)
	{
	  int curval = THREAD_ATOMIC_CMPXCHG_VAL (self, cancelhandling,
						  cancelhandling
						  | CANCELTYPE_BITMASK,
						  cancelhandling);
	  if (__builtin_expect (curval == cancelhandling, 1))
	    /* Successfully replaced the value.  */
	    break;

	  /* Prepare for the next round.  */
	  cancelhandling = curval;
	}

      CANCELLATION_P (self);
    }
}
