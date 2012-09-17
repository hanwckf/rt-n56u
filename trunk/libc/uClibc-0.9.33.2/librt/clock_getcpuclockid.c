/* clock_getcpuclockid -- Get a clockid_t for process CPU time.  Linux version.
   Copyright (C) 2004, 2005 Free Software Foundation, Inc.
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

#include <errno.h>
#include <time.h>
#include <sysdep.h>
#include <unistd.h>
#include <bits/kernel-features.h>
#include "kernel-posix-cpu-timers.h"

#ifndef HAS_CPUCLOCK
# define HAS_CPUCLOCK 1
#endif

int
clock_getcpuclockid (pid_t pid, clockid_t *clock_id)
{
#ifdef __NR_clock_getres
  /* The clockid_t value is a simple computation from the PID.
     But we do a clock_getres call to validate it.  */

  const clockid_t pidclock = MAKE_PROCESS_CPUCLOCK (pid, CPUCLOCK_SCHED);

# if !(__ASSUME_POSIX_CPU_TIMERS > 0)
  extern int __libc_missing_posix_cpu_timers attribute_hidden;
#  if !(__ASSUME_POSIX_TIMERS > 0)
  extern int __libc_missing_posix_timers attribute_hidden;
  if (__libc_missing_posix_timers && !__libc_missing_posix_cpu_timers)
    __libc_missing_posix_cpu_timers = 1;
#  endif
  if (!__libc_missing_posix_cpu_timers)
# endif
    {
      INTERNAL_SYSCALL_DECL (err);
      int r = INTERNAL_SYSCALL (clock_getres, err, 2, pidclock, NULL);
      if (!INTERNAL_SYSCALL_ERROR_P (r, err))
	{
	  *clock_id = pidclock;
	  return 0;
	}

# if !(__ASSUME_POSIX_TIMERS > 0)
      if (INTERNAL_SYSCALL_ERRNO (r, err) == ENOSYS)
	{
	  /* The kernel doesn't support these calls at all.  */
	  __libc_missing_posix_timers = 1;
	  __libc_missing_posix_cpu_timers = 1;
	}
      else
# endif
	if (INTERNAL_SYSCALL_ERRNO (r, err) == EINVAL)
	  {
# if !(__ASSUME_POSIX_CPU_TIMERS > 0)
	    if (pidclock == MAKE_PROCESS_CPUCLOCK (0, CPUCLOCK_SCHED)
		|| INTERNAL_SYSCALL_ERROR_P (INTERNAL_SYSCALL
					     (clock_getres, err, 2,
					      MAKE_PROCESS_CPUCLOCK
					      (0, CPUCLOCK_SCHED), NULL),
					     err))
	      /* The kernel doesn't support these clocks at all.  */
	      __libc_missing_posix_cpu_timers = 1;
	    else
# endif
	      /* The clock_getres system call checked the PID for us.  */
	      return ESRCH;
	  }
	else
	  return INTERNAL_SYSCALL_ERRNO (r, err);
    }
#endif

  /* We don't allow any process ID but our own.  */
  if (pid != 0 && pid != getpid ())
    return EPERM;

#ifdef CLOCK_PROCESS_CPUTIME_ID
  if (HAS_CPUCLOCK)
    {
      /* Store the number.  */
      *clock_id = CLOCK_PROCESS_CPUTIME_ID;

      return 0;
    }
#endif

  /* We don't have a timer for that.  */
  return ENOENT;
}
