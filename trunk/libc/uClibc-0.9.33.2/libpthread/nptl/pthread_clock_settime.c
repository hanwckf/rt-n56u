/* Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <time.h>
#include <libc-internal.h>
#include "pthreadP.h"


#if HP_TIMING_AVAIL
int
__pthread_clock_settime (clockid_t clock_id, hp_timing_t offset)
{
  /* This is the ID of the thread we are looking for.  */
  pid_t tid = ((unsigned int) clock_id) >> CLOCK_IDFIELD_SIZE;

  /* Compute the offset since the start time of the process.  */
  if (tid == 0 || tid == THREAD_GETMEM (THREAD_SELF, tid))
    /* Our own clock.  */
    THREAD_SETMEM (THREAD_SELF, cpuclock_offset, offset);
  else
    {
      /* This is more complicated.  We have to locate the thread based
	 on the ID.  This means walking the list of existing
	 threads.  */
      struct pthread *thread = __find_thread_by_id (tid);
      if (thread == NULL)
	{
	  __set_errno (EINVAL);
	  return -1;
	}

      /* There is a race here.  The thread might terminate and the stack
	 become unusable.  But this is the user's problem.  */
      thread->cpuclock_offset = offset;
    }

  return 0;
}
#endif
