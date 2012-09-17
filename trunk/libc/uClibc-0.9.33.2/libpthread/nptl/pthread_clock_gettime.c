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
__pthread_clock_gettime (clockid_t clock_id, hp_timing_t freq,
			 struct timespec *tp)
{
  hp_timing_t tsc;

  /* Get the current counter.  */
  HP_TIMING_NOW (tsc);

  /* This is the ID of the thread we are looking for.  */
  pid_t tid = ((unsigned int) clock_id) >> CLOCK_IDFIELD_SIZE;

  /* Compute the offset since the start time of the process.  */
  if (tid == 0 || tid == THREAD_GETMEM (THREAD_SELF, tid))
    /* Our own clock.  */
    tsc -= THREAD_GETMEM (THREAD_SELF, cpuclock_offset);
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
      tsc -= thread->cpuclock_offset;
    }

  /* Compute the seconds.  */
  tp->tv_sec = tsc / freq;

  /* And the nanoseconds.  This computation should be stable until
     we get machines with about 16GHz frequency.  */
  tp->tv_nsec = ((tsc % freq) * 1000000000ull) / freq;

  return 0;
}
#endif
