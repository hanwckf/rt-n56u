/* Copyright (C) 2003, 2004, 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <sysdep.h>
#include <bits/kernel-features.h>
#include <pthreadP.h>
#include "kernel-posix-timers.h"


/* List of active SIGEV_THREAD timers.  */
struct timer *__active_timer_sigev_thread;
/* Lock for the __active_timer_sigev_thread.  */
pthread_mutex_t __active_timer_sigev_thread_lock = PTHREAD_MUTEX_INITIALIZER;


struct thread_start_data
{
  void (*thrfunc) (sigval_t);
  sigval_t sival;
};


#ifdef __NR_timer_create
/* Helper thread to call the user-provided function.  */
static void *
timer_sigev_thread (void *arg)
{
  /* The parent thread has all signals blocked.  This is a bit
     surprising for user code, although valid.  We unblock all
     signals.  */
  sigset_t ss;
  __sigemptyset (&ss);
  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL (rt_sigprocmask, err, 4, SIG_SETMASK, &ss, NULL, _NSIG / 8);

  struct thread_start_data *td = (struct thread_start_data *) arg;

  void (*thrfunc) (sigval_t) = td->thrfunc;
  sigval_t sival = td->sival;

  /* The TD object was allocated in timer_helper_thread.  */
  free (td);

  /* Call the user-provided function.  */
  thrfunc (sival);

  return NULL;
}


/* Helper function to support starting threads for SIGEV_THREAD.  */
static attribute_noreturn void *
timer_helper_thread (void *arg)
{
  /* Wait for the SIGTIMER signal, allowing the setXid signal, and
     none else.  */
  sigset_t ss;
  __sigemptyset (&ss);
  __sigaddset (&ss, SIGTIMER);

  /* Endless loop of waiting for signals.  The loop is only ended when
     the thread is canceled.  */
  while (1)
    {
      siginfo_t si;

      /* sigwaitinfo cannot be used here, since it deletes
	 SIGCANCEL == SIGTIMER from the set.  */

      int oldtype = LIBC_CANCEL_ASYNC ();

      /* XXX The size argument hopefully will have to be changed to the
	 real size of the user-level sigset_t.  */
      int result = INLINE_SYSCALL (rt_sigtimedwait, 4, &ss, &si, NULL,
				   _NSIG / 8);

      LIBC_CANCEL_RESET (oldtype);

      if (result > 0)
	{
	  if (si.si_code == SI_TIMER)
	    {
	      struct timer *tk = (struct timer *) si.si_ptr;

	      /* Check the timer is still used and will not go away
		 while we are reading the values here.  */
	      pthread_mutex_lock (&__active_timer_sigev_thread_lock);

	      struct timer *runp = __active_timer_sigev_thread;
	      while (runp != NULL)
		if (runp == tk)
		  break;
		else
		  runp = runp->next;

	      if (runp != NULL)
		{
		  struct thread_start_data *td = malloc (sizeof (*td));

		  /* There is not much we can do if the allocation fails.  */
		  if (td != NULL)
		    {
		      /* This is the signal we are waiting for.  */
		      td->thrfunc = tk->thrfunc;
		      td->sival = tk->sival;

		      pthread_t th;
		      (void) pthread_create (&th, &tk->attr,
					     timer_sigev_thread, td);
		    }
		}

	      pthread_mutex_unlock (&__active_timer_sigev_thread_lock);
	    }
	  else if (si.si_code == SI_TKILL)
	    /* The thread is canceled.  */
	    pthread_exit (NULL);
	}
    }
}


/* Control variable for helper thread creation.  */
pthread_once_t __helper_once attribute_hidden;


/* TID of the helper thread.  */
pid_t __helper_tid attribute_hidden;


/* Reset variables so that after a fork a new helper thread gets started.  */
static void
reset_helper_control (void)
{
  __helper_once = PTHREAD_ONCE_INIT;
  __helper_tid = 0;
}


void
attribute_hidden
__start_helper_thread (void)
{
  /* The helper thread needs only very little resources
     and should go away automatically when canceled.  */
  pthread_attr_t attr;
  (void) pthread_attr_init (&attr);
  (void) pthread_attr_setstacksize (&attr, PTHREAD_STACK_MIN);

  /* Block all signals in the helper thread but SIGSETXID.  To do this
     thoroughly we temporarily have to block all signals here.  The
     helper can lose wakeups if SIGCANCEL is not blocked throughout,
     but sigfillset omits it SIGSETXID.  So, we add SIGCANCEL back
     explicitly here.  */
  sigset_t ss;
  sigset_t oss;
  sigfillset (&ss);
  __sigaddset (&ss, SIGCANCEL);
  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL (rt_sigprocmask, err, 4, SIG_SETMASK, &ss, &oss, _NSIG / 8);

  /* Create the helper thread for this timer.  */
  pthread_t th;
  int res = pthread_create (&th, &attr, timer_helper_thread, NULL);
  if (res == 0)
    /* We managed to start the helper thread.  */
    __helper_tid = ((struct pthread *) th)->tid;

  /* Restore the signal mask.  */
  INTERNAL_SYSCALL (rt_sigprocmask, err, 4, SIG_SETMASK, &oss, NULL,
		    _NSIG / 8);

  /* No need for the attribute anymore.  */
  (void) pthread_attr_destroy (&attr);

  /* We have to make sure that after fork()ing a new helper thread can
     be created.  */
  pthread_atfork (NULL, NULL, reset_helper_control);
}
#endif

#ifndef __ASSUME_POSIX_TIMERS
# include <nptl/sysdeps/pthread/timer_routines.c>
#endif
