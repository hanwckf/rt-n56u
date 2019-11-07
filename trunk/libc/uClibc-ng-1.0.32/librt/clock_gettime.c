/* clock_gettime -- Get current time from a POSIX clockid_t.  Linux version.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <sysdep.h>
#include <time.h>
#include <sys/time.h>
#include "kernel-posix-cpu-timers.h"


#define SYSCALL_GETTIME \
  retval = INLINE_SYSCALL (clock_gettime, 2, clock_id, tp); \
  break

/* The REALTIME and MONOTONIC clock are definitely supported in the kernel.  */
#define SYSDEP_GETTIME							      \
  SYSDEP_GETTIME_CPUTIME						      \
  case CLOCK_REALTIME:							      \
  case CLOCK_MONOTONIC:							      \
    SYSCALL_GETTIME

/* We handled the REALTIME clock here.  */
#define HANDLED_REALTIME	1
#define HANDLED_CPUTIME	1

#define SYSDEP_GETTIME_CPU SYSCALL_GETTIME
#define SYSDEP_GETTIME_CPUTIME	/* Default catches them too.  */

static inline int
realtime_gettime (struct timespec *tp)
{
  struct timeval tv;
  int retval = gettimeofday (&tv, NULL);
  if (retval == 0)
    /* Convert into `timespec'.  */
    TIMEVAL_TO_TIMESPEC (&tv, tp);
  return retval;
}

/* Get current value of CLOCK and store it in TP.  */
int
clock_gettime (clockid_t clock_id, struct timespec *tp)
{
  int retval = -1;
#ifndef HANDLED_REALTIME
  struct timeval tv;
#endif

  switch (clock_id)
    {
#ifdef SYSDEP_GETTIME
      SYSDEP_GETTIME;
#endif

#ifndef HANDLED_REALTIME
    case CLOCK_REALTIME:
      retval = gettimeofday (&tv, NULL);
      if (retval == 0)
	TIMEVAL_TO_TIMESPEC (&tv, tp);
      break;
#endif

    default:
#ifdef SYSDEP_GETTIME_CPU
      SYSDEP_GETTIME_CPU;
#endif
	__set_errno (EINVAL);
      break;
    }

  return retval;
}
