/* Copyright (C) 2003, 2007 Free Software Foundation, Inc.
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


#ifdef __NR_timer_delete
# ifndef __ASSUME_POSIX_TIMERS
static int compat_timer_delete (timer_t timerid);
#  define timer_delete static compat_timer_delete
#  include <nptl/sysdeps/pthread/timer_delete.c>
#  undef timer_delete
# endif

# ifdef timer_delete_alias
#  define timer_delete timer_delete_alias
# endif


int
timer_delete (
     timer_t timerid)
{
# undef timer_delete
# ifndef __ASSUME_POSIX_TIMERS
  if (__no_posix_timers >= 0)
# endif
    {
      struct timer *kt = (struct timer *) timerid;

      /* Delete the kernel timer object.  */
      int res = INLINE_SYSCALL (timer_delete, 1, kt->ktimerid);

      if (res == 0)
	{
	  if (kt->sigev_notify == SIGEV_THREAD)
	    {
	      /* Remove the timer from the list.  */
	      pthread_mutex_lock (&__active_timer_sigev_thread_lock);
	      if (__active_timer_sigev_thread == kt)
		__active_timer_sigev_thread = kt->next;
	      else
		{
		  struct timer *prevp = __active_timer_sigev_thread;
		  while (prevp->next != NULL)
		    if (prevp->next == kt)
		      {
			prevp->next = kt->next;
			break;
		      }
		    else
		      prevp = prevp->next;
		}
	      pthread_mutex_unlock (&__active_timer_sigev_thread_lock);
	    }

# ifndef __ASSUME_POSIX_TIMERS
	  /* We know the syscall support is available.  */
	  __no_posix_timers = 1;
# endif

	  /* Free the memory.  */
	  (void) free (kt);

	  return 0;
	}

      /* The kernel timer is not known or something else bad happened.
	 Return the error.  */
# ifndef __ASSUME_POSIX_TIMERS
      if (errno != ENOSYS)
	{
	  __no_posix_timers = 1;
# endif
	  return -1;
# ifndef __ASSUME_POSIX_TIMERS
	}

      __no_posix_timers = -1;
# endif
    }

# ifndef __ASSUME_POSIX_TIMERS
  return compat_timer_delete (timerid);
# endif
}
#else
# ifdef timer_delete_alias
#  define timer_delete timer_delete_alias
# endif
/* The new system calls are not available.  Use the userlevel
   implementation.  */
# include <nptl/sysdeps/pthread/timer_delete.c>
#endif
