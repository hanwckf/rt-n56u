/* Copyright (C) 2001, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <time.h>
#include <libc-internal.h>
#include "internals.h"
#include "spinlock.h"


#if HP_TIMING_AVAIL
int
__pthread_clock_settime (clockid_t clock_id, hp_timing_t offset)
{
  pthread_descr self = thread_self ();
  pthread_t thread = ((unsigned int) clock_id) >> CLOCK_IDFIELD_SIZE;
  const unsigned int mask = ~0U >> CLOCK_IDFIELD_SIZE;

  if (thread == 0 || (THREAD_GETMEM (self, p_tid) & mask) == thread)
    /* Our own clock.  */
    THREAD_SETMEM (self, p_cpuclock_offset, offset);
  else
    {
      pthread_descr th;
      pthread_handle handle = thread_handle (thread);
      __pthread_lock (&handle->h_lock, NULL);
      th = handle->h_descr;
      if (th == NULL || (th->p_tid & mask) != thread || th->p_terminated)
	{
	  __pthread_unlock (&handle->h_lock);
	  __set_errno (EINVAL);
	  return -1;
	}
      th->p_cpuclock_offset = offset;
      __pthread_unlock (&handle->h_lock);
   }

  return 0;
}
#endif
