/* Copyright (C) 2003, 2004, 2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <time.h>
#include <errno.h>

#include <sysdep-cancel.h>
#include <bits/kernel-features.h>
#include "kernel-posix-cpu-timers.h"


#ifdef __ASSUME_POSIX_TIMERS
/* We can simply use the syscall.  The CPU clocks are not supported
   with this function.  */
int
clock_nanosleep (clockid_t clock_id, int flags, const struct timespec *req,
		 struct timespec *rem)
{
  INTERNAL_SYSCALL_DECL (err);
  int r;

  if (clock_id == CLOCK_THREAD_CPUTIME_ID)
    return EINVAL;
  if (clock_id == CLOCK_PROCESS_CPUTIME_ID)
    clock_id = MAKE_PROCESS_CPUCLOCK (0, CPUCLOCK_SCHED);

  if (SINGLE_THREAD_P)
    r = INTERNAL_SYSCALL (clock_nanosleep, err, 4, clock_id, flags, req, rem);
  else
    {
      int oldstate = LIBC_CANCEL_ASYNC ();

      r = INTERNAL_SYSCALL (clock_nanosleep, err, 4, clock_id, flags, req,
			    rem);

      LIBC_CANCEL_RESET (oldstate);
    }

  return (INTERNAL_SYSCALL_ERROR_P (r, err)
	  ? INTERNAL_SYSCALL_ERRNO (r, err) : 0);
}

#else
# ifdef __NR_clock_nanosleep
/* Is the syscall known to exist?  */
extern int __libc_missing_posix_timers attribute_hidden;

/* The REALTIME and MONOTONIC clock might be available.  Try the
   syscall first.  */
#  define SYSDEP_NANOSLEEP \
  if (!__libc_missing_posix_timers)					      \
    {									      \
      clockid_t syscall_clockid;					      \
      INTERNAL_SYSCALL_DECL (err);					      \
									      \
      if (clock_id == CLOCK_THREAD_CPUTIME_ID)				      \
	return EINVAL;							      \
      if (clock_id == CLOCK_PROCESS_CPUTIME_ID)				      \
	syscall_clockid = MAKE_PROCESS_CPUCLOCK (0, CPUCLOCK_SCHED);	      \
      else								      \
	syscall_clockid = clock_id;					      \
									      \
      int oldstate = LIBC_CANCEL_ASYNC ();				      \
									      \
      int r = INTERNAL_SYSCALL (clock_nanosleep, err, 4,		      \
				syscall_clockid, flags, req, rem);	      \
									      \
      LIBC_CANCEL_RESET (oldstate);					      \
									      \
      if (!INTERNAL_SYSCALL_ERROR_P (r, err))				      \
	return 0;							      \
									      \
      if (INTERNAL_SYSCALL_ERRNO (r, err) != ENOSYS)			      \
	return INTERNAL_SYSCALL_ERRNO (r, err);				      \
									      \
      __libc_missing_posix_timers = 1;					      \
    }
# endif

# include <sysdeps/unix/clock_nanosleep.c>
#endif
