/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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
#include <sysdep.h>
#include <bits/kernel-features.h>
#include "kernel-posix-timers.h"


#ifdef __NR_timer_settime
# ifndef __ASSUME_POSIX_TIMERS
static int compat_timer_settime (timer_t timerid, int flags,
				 const struct itimerspec *value,
				 struct itimerspec *ovalue);
#  define timer_settime static compat_timer_settime
#  include <nptl/sysdeps/pthread/timer_settime.c>
#  undef timer_settime
# endif

# ifdef timer_settime_alias
#  define timer_settime timer_settime_alias
# endif


int
timer_settime (
     timer_t timerid,
     int flags,
     const struct itimerspec *value,
     struct itimerspec *ovalue)
{
# undef timer_settime
# ifndef __ASSUME_POSIX_TIMERS
  if (__no_posix_timers >= 0)
# endif
    {
      struct timer *kt = (struct timer *) timerid;

      /* Delete the kernel timer object.  */
      int res = INLINE_SYSCALL (timer_settime, 4, kt->ktimerid, flags,
				value, ovalue);

# ifndef __ASSUME_POSIX_TIMERS
      if (res != -1 || errno != ENOSYS)
	{
	  /* We know the syscall support is available.  */
	  __no_posix_timers = 1;
# endif
	  return res;
# ifndef __ASSUME_POSIX_TIMERS
	}
# endif

# ifndef __ASSUME_POSIX_TIMERS
      __no_posix_timers = -1;
# endif
    }

# ifndef __ASSUME_POSIX_TIMERS
  return compat_timer_settime (timerid, flags, value, ovalue);
# endif
}
#else
# ifdef timer_settime_alias
#  define timer_settime timer_settime_alias
# endif
/* The new system calls are not available.  Use the userlevel
   implementation.  */
# include <nptl/sysdeps/pthread/timer_settime.c>
#endif
