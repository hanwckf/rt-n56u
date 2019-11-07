/* Copyright (C) 2003,2004, 2007, 2009 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sysdep.h>
#include <bits/kernel-features.h>
#include <internaltypes.h>
#include <pthreadP.h>
#include "kernel-posix-timers.h"
#include "kernel-posix-cpu-timers.h"


#ifdef __NR_timer_create
# ifndef __ASSUME_POSIX_TIMERS
static int compat_timer_create (clockid_t clock_id, struct sigevent *evp,
				timer_t *timerid);
#  define timer_create static compat_timer_create
#  include <nptl/sysdeps/pthread/timer_create.c>
#  undef timer_create

/* Nonzero if the system calls are not available.  */
int __no_posix_timers attribute_hidden;
# endif

# ifdef timer_create_alias
#  define timer_create timer_create_alias
# endif


int
timer_create (
     clockid_t clock_id,
     struct sigevent *evp,
     timer_t *timerid)
{
# undef timer_create
# ifndef __ASSUME_POSIX_TIMERS
  if  (__no_posix_timers >= 0)
# endif
    {
      clockid_t syscall_clockid = (clock_id == CLOCK_PROCESS_CPUTIME_ID
				   ? MAKE_PROCESS_CPUCLOCK (0, CPUCLOCK_SCHED)
				   : clock_id == CLOCK_THREAD_CPUTIME_ID
				   ? MAKE_THREAD_CPUCLOCK (0, CPUCLOCK_SCHED)
				   : clock_id);

      /* If the user wants notification via a thread we need to handle
	 this special.  */
      if (evp == NULL
	  || __builtin_expect (evp->sigev_notify != SIGEV_THREAD, 1))
	{
	  struct sigevent local_evp;

	  /* We avoid allocating too much memory by basically
	     using struct timer as a derived class with the
	     first two elements being in the superclass.  We only
	     need these two elements here.  */
	  struct timer *newp = (struct timer *) malloc (offsetof (struct timer,
								  thrfunc));
	  if (newp == NULL)
	    /* No more memory.  */
	    return -1;

	  if (evp == NULL)
	    {
	      /* The kernel has to pass up the timer ID which is a
		 userlevel object.  Therefore we cannot leave it up to
		 the kernel to determine it.  */
	      local_evp.sigev_notify = SIGEV_SIGNAL;
	      local_evp.sigev_signo = SIGALRM;
	      local_evp.sigev_value.sival_ptr = newp;

	      evp = &local_evp;
	    }

	  kernel_timer_t ktimerid;
	  int retval = INLINE_SYSCALL (timer_create, 3, syscall_clockid, evp,
				       &ktimerid);

# ifndef __ASSUME_POSIX_TIMERS
	  if (retval != -1 || errno != ENOSYS)
# endif
	    {
# ifndef __ASSUME_POSIX_TIMERS
	      __no_posix_timers = 1;
# endif

	      if (retval != -1)
		{
		  newp->sigev_notify = (evp != NULL
					? evp->sigev_notify : SIGEV_SIGNAL);
		  newp->ktimerid = ktimerid;

		  *timerid = (timer_t) newp;
		}
	      else
		{
		  /* Cannot allocate the timer, fail.  */
		  free (newp);
		  retval = -1;
		}

	      return retval;
	    }

	  free (newp);

# ifndef __ASSUME_POSIX_TIMERS
	  /* When we come here the syscall does not exist.  Make sure we
	     do not try to use it again.  */
	  __no_posix_timers = -1;
# endif
	}
      else
	{
# ifndef __ASSUME_POSIX_TIMERS
	  /* Make sure we have the necessary kernel support.  */
	  if (__no_posix_timers == 0)
	    {
	      INTERNAL_SYSCALL_DECL (err);
	      struct timespec ts;
	      int res;
	      res = INTERNAL_SYSCALL (clock_getres, err, 2,
				      CLOCK_REALTIME, &ts);
	      __no_posix_timers = (INTERNAL_SYSCALL_ERROR_P (res, err)
				   ? -1 : 1);
	    }

	  if (__no_posix_timers > 0)
# endif
	    {
	      /* Create the helper thread.  */
	      pthread_once (&__helper_once, __start_helper_thread);
	      if (__helper_tid == 0)
		{
		  /* No resources to start the helper thread.  */
		  __set_errno (EAGAIN);
		  return -1;
		}

	      struct timer *newp;
	      newp = (struct timer *) malloc (sizeof (struct timer));
	      if (newp == NULL)
		return -1;

	      /* Copy the thread parameters the user provided.  */
	      newp->sival = evp->sigev_value;
	      newp->thrfunc = evp->sigev_notify_function;
	      newp->sigev_notify = SIGEV_THREAD;

	      /* We cannot simply copy the thread attributes since the
		 implementation might keep internal information for
		 each instance.  */
	      (void) pthread_attr_init (&newp->attr);
	      if (evp->sigev_notify_attributes != NULL)
		{
		  struct pthread_attr *nattr;
		  struct pthread_attr *oattr;

		  nattr = (struct pthread_attr *) &newp->attr;
		  oattr = (struct pthread_attr *) evp->sigev_notify_attributes;

		  nattr->schedparam = oattr->schedparam;
		  nattr->schedpolicy = oattr->schedpolicy;
		  nattr->flags = oattr->flags;
		  nattr->guardsize = oattr->guardsize;
		  nattr->stackaddr = oattr->stackaddr;
		  nattr->stacksize = oattr->stacksize;
		}

	      /* In any case set the detach flag.  */
	      (void) pthread_attr_setdetachstate (&newp->attr,
						  PTHREAD_CREATE_DETACHED);

	      /* Create the event structure for the kernel timer.  */
	      struct sigevent sev =
		{ .sigev_value.sival_ptr = newp,
		  .sigev_signo = SIGTIMER,
		  .sigev_notify = SIGEV_SIGNAL | SIGEV_THREAD_ID,
		  ._sigev_un = { ._pad = { [0] = __helper_tid } } };

	      /* Create the timer.  */
	      INTERNAL_SYSCALL_DECL (err);
	      int res;
	      res = INTERNAL_SYSCALL (timer_create, err, 3,
				      syscall_clockid, &sev, &newp->ktimerid);
	      if (! INTERNAL_SYSCALL_ERROR_P (res, err))
		{
		  /* Add to the queue of active timers with thread
		     delivery.  */
		  pthread_mutex_lock (&__active_timer_sigev_thread_lock);
		  newp->next = __active_timer_sigev_thread;
		  __active_timer_sigev_thread = newp;
		  pthread_mutex_unlock (&__active_timer_sigev_thread_lock);

		  *timerid = (timer_t) newp;
		  return 0;
		}

	      /* Free the resources.  */
	      free (newp);

	      __set_errno (INTERNAL_SYSCALL_ERRNO (res, err));

	      return -1;
	    }
	}
    }

# ifndef __ASSUME_POSIX_TIMERS
  /* Compatibility code.  */
  return compat_timer_create (clock_id, evp, timerid);
# endif
}
#else
# ifdef timer_create_alias
#  define timer_create timer_create_alias
# endif
/* The new system calls are not available.  Use the userlevel
   implementation.  */
# include <nptl/sysdeps/pthread/timer_create.c>
#endif
