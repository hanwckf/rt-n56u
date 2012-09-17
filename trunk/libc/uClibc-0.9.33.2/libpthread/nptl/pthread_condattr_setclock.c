/* Copyright (C) 2003, 2004, 2007, 2008 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include <sysdep.h>
#include "pthreadP.h"
#include <bits/kernel-features.h>


int
pthread_condattr_setclock (
     pthread_condattr_t *attr,
     clockid_t clock_id)
{
  /* Only a few clocks are allowed.  CLOCK_REALTIME is always allowed.
     CLOCK_MONOTONIC only if the kernel has the necessary support.  */
  if (clock_id == CLOCK_MONOTONIC)
    {
#ifndef __ASSUME_POSIX_TIMERS
# ifdef __NR_clock_getres
      /* Check whether the clock is available.  */
      static int avail;

      if (avail == 0)
	{
	  struct timespec ts;

	  INTERNAL_SYSCALL_DECL (err);
	  int val;
	  val = INTERNAL_SYSCALL (clock_getres, err, 2, CLOCK_MONOTONIC, &ts);
	  avail = INTERNAL_SYSCALL_ERROR_P (val, err) ? -1 : 1;
	}

      if (avail < 0)
# endif
	/* Not available.  */
	return EINVAL;
#endif
    }
  else if (clock_id != CLOCK_REALTIME)
    /* If more clocks are allowed some day the storing of the clock ID
       in the pthread_cond_t structure needs to be adjusted.  */
    return EINVAL;

  /* Make sure the value fits in the bits we reserved.  */
  assert (clock_id < (1 << COND_NWAITERS_SHIFT));

  int *valuep = &((struct pthread_condattr *) attr)->value;

  *valuep = ((*valuep & ~(((1 << COND_NWAITERS_SHIFT) - 1) << 1))
	     | (clock_id << 1));

  return 0;
}
