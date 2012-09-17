/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Signal handlers */

#include "internals.h"


/* The wrapper around user-provided signal handlers */
void __pthread_sighandler(int signo, SIGCONTEXT ctx)
{
  pthread_descr self;
  char * in_sighandler;
  self = check_thread_self();

  /* If we're in a sigwait operation, just record the signal received
     and return without calling the user's handler */
  if (THREAD_GETMEM(self, p_sigwaiting)) {
    THREAD_SETMEM(self, p_sigwaiting, 0);
    THREAD_SETMEM(self, p_signal, signo);
    return;
  }
  /* Record that we're in a signal handler and call the user's
     handler function */
  in_sighandler = THREAD_GETMEM(self, p_in_sighandler);
  if (in_sighandler == NULL)
    THREAD_SETMEM(self, p_in_sighandler, CURRENT_STACK_FRAME);
  CALL_SIGHANDLER(__sighandler[signo].old, signo, ctx);
  if (in_sighandler == NULL)
    THREAD_SETMEM(self, p_in_sighandler, NULL);
}

/* The same, this time for real-time signals.  */
void __pthread_sighandler_rt(int signo, struct siginfo *si,
			     struct ucontext *uc)
{
  pthread_descr self;
  char * in_sighandler;
  self = check_thread_self();

  /* If we're in a sigwait operation, just record the signal received
     and return without calling the user's handler */
  if (THREAD_GETMEM(self, p_sigwaiting)) {
    THREAD_SETMEM(self, p_sigwaiting, 0);
    THREAD_SETMEM(self, p_signal, signo);
    return;
  }
  /* Record that we're in a signal handler and call the user's
     handler function */
  in_sighandler = THREAD_GETMEM(self, p_in_sighandler);
  if (in_sighandler == NULL)
    THREAD_SETMEM(self, p_in_sighandler, CURRENT_STACK_FRAME);
  __sighandler[signo].rt(signo, si, uc);
  if (in_sighandler == NULL)
    THREAD_SETMEM(self, p_in_sighandler, NULL);
}


/* A signal handler that does nothing */
void __pthread_null_sighandler(int sig) { }
