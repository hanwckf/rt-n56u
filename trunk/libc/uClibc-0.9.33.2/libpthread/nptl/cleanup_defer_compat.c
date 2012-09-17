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

#include "pthreadP.h"


void
attribute_protected
__pthread_cleanup_push_defer (
     struct _pthread_cleanup_buffer *buffer,
     void (*routine) (void *),
     void *arg)
{
  struct pthread *self = THREAD_SELF;

  buffer->__routine = routine;
  buffer->__arg = arg;
  buffer->__prev = THREAD_GETMEM (self, cleanup);

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

  buffer->__canceltype = (cancelhandling & CANCELTYPE_BITMASK
			  ? PTHREAD_CANCEL_ASYNCHRONOUS
			  : PTHREAD_CANCEL_DEFERRED);

  THREAD_SETMEM (self, cleanup, buffer);
}
strong_alias (__pthread_cleanup_push_defer, _pthread_cleanup_push_defer)


void
attribute_protected
__pthread_cleanup_pop_restore (
     struct _pthread_cleanup_buffer *buffer,
     int execute)
{
  struct pthread *self = THREAD_SELF;

  THREAD_SETMEM (self, cleanup, buffer->__prev);

  int cancelhandling;
  if (__builtin_expect (buffer->__canceltype != PTHREAD_CANCEL_DEFERRED, 0)
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

  /* If necessary call the cleanup routine after we removed the
     current cleanup block from the list.  */
  if (execute)
    buffer->__routine (buffer->__arg);
}
strong_alias (__pthread_cleanup_pop_restore, _pthread_cleanup_pop_restore)
