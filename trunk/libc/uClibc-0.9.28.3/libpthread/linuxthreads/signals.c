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

/* Handling of signals */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include <ucontext.h>
#include <bits/sigcontextinfo.h>

/* mods for uClibc: __libc_sigaction is not in any standard headers */
extern int __libc_sigaction (int sig, const struct sigaction *act, struct sigaction *oact);

int pthread_sigmask(int how, const sigset_t * newmask, sigset_t * oldmask)
{
  sigset_t mask;

  if (newmask != NULL) {
    mask = *newmask;
    /* Don't allow __pthread_sig_restart to be unmasked.
       Don't allow __pthread_sig_cancel to be masked. */
    switch(how) {
    case SIG_SETMASK:
      sigaddset(&mask, __pthread_sig_restart);
      sigdelset(&mask, __pthread_sig_cancel);
      if (__pthread_sig_debug > 0)
	sigdelset(&mask, __pthread_sig_debug);
      break;
    case SIG_BLOCK:
      sigdelset(&mask, __pthread_sig_cancel);
      if (__pthread_sig_debug > 0)
	sigdelset(&mask, __pthread_sig_debug);
      break;
    case SIG_UNBLOCK:
      sigdelset(&mask, __pthread_sig_restart);
      break;
    }
    newmask = &mask;
  }
  if (sigprocmask(how, newmask, oldmask) == -1)
    return errno;
  else
    return 0;
}

int pthread_kill(pthread_t thread, int signo)
{
  pthread_handle handle = thread_handle(thread);
  int pid;

  __pthread_lock(&handle->h_lock, NULL);
  if (invalid_handle(handle, thread)) {
    __pthread_unlock(&handle->h_lock);
    return ESRCH;
  }
  pid = handle->h_descr->p_pid;
  __pthread_unlock(&handle->h_lock);
  if (kill(pid, signo) == -1)
    return errno;
  else
    return 0;
}

/* User-provided signal handlers */
typedef void (*arch_sighandler_t) __PMT ((int, SIGCONTEXT));
static union
{
  arch_sighandler_t old;
  void (*rt) (int, struct siginfo *, struct ucontext *);
} sighandler[NSIG];

/* The wrapper around user-provided signal handlers */
static void pthread_sighandler(int signo, SIGCONTEXT ctx)
{
  pthread_descr self = thread_self();
  char * in_sighandler;
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
  sighandler[signo].old(signo, SIGCONTEXT_EXTRA_ARGS ctx);
  if (in_sighandler == NULL)
    THREAD_SETMEM(self, p_in_sighandler, NULL);
}

/* The same, this time for real-time signals.  */
static void pthread_sighandler_rt(int signo, struct siginfo *si,
				  struct ucontext *uc)
{
  pthread_descr self = thread_self();
  char * in_sighandler;
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
  sighandler[signo].rt(signo, si, uc);
  if (in_sighandler == NULL)
    THREAD_SETMEM(self, p_in_sighandler, NULL);
}

/* The wrapper around sigaction.  Install our own signal handler
   around the signal. */
int __sigaction(int sig, const struct sigaction * act,
              struct sigaction * oact)
{
  struct sigaction newact;
  struct sigaction *newactp;

#ifdef DEBUG_PT
printf(__FUNCTION__": pthreads wrapper!\n");
#endif
  if (sig == __pthread_sig_restart ||
      sig == __pthread_sig_cancel ||
      (sig == __pthread_sig_debug && __pthread_sig_debug > 0))
    return EINVAL;
  if (act)
    {
      newact = *act;
      if (act->sa_handler != SIG_IGN && act->sa_handler != SIG_DFL
	  && sig > 0 && sig < NSIG)
	{
	  if (act->sa_flags & SA_SIGINFO)
	    newact.sa_handler = (__sighandler_t) pthread_sighandler_rt;
	  else
	    newact.sa_handler = (__sighandler_t) pthread_sighandler;
	}
      newactp = &newact;
    }
  else
    newactp = NULL;
  if (__libc_sigaction(sig, newactp, oact) == -1)
    return -1;
#ifdef DEBUG_PT
printf(__FUNCTION__": signahdler installed, __sigaction successful\n");
#endif
  if (sig > 0 && sig < NSIG)
    {
      if (oact != NULL)
	oact->sa_handler = (__sighandler_t) sighandler[sig].old;
      if (act)
	/* For the assignment is does not matter whether it's a normal
	   or real-time signal.  */
	sighandler[sig].old = (arch_sighandler_t) act->sa_handler;
    }
  return 0;
}
strong_alias(__sigaction, sigaction)

/* A signal handler that does nothing */
static void pthread_null_sighandler(int sig) { }

/* sigwait -- synchronously wait for a signal */
int sigwait(const sigset_t * set, int * sig)
{
  volatile pthread_descr self = thread_self();
  sigset_t mask;
  int s;
  sigjmp_buf jmpbuf;
  struct sigaction sa;

  /* Get ready to block all signals except those in set
     and the cancellation signal.
     Also check that handlers are installed on all signals in set,
     and if not, install our dummy handler.  This is conformant to
     POSIX: "The effect of sigwait() on the signal actions for the
     signals in set is unspecified." */
  sigfillset(&mask);
  sigdelset(&mask, __pthread_sig_cancel);
  for (s = 1; s <= NSIG; s++) {
    if (sigismember(set, s) &&
        s != __pthread_sig_restart &&
        s != __pthread_sig_cancel &&
        s != __pthread_sig_debug) {
      sigdelset(&mask, s);
      if (sighandler[s].old == NULL ||
	  sighandler[s].old == (arch_sighandler_t) SIG_DFL ||
	  sighandler[s].old == (arch_sighandler_t) SIG_IGN) {
        sa.sa_handler = pthread_null_sighandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(s, &sa, NULL);
      }
    }
  }
  /* Test for cancellation */
  if (sigsetjmp(jmpbuf, 1) == 0) {
    THREAD_SETMEM(self, p_cancel_jmp, &jmpbuf);
    if (! (THREAD_GETMEM(self, p_canceled)
	   && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE)) {
      /* Reset the signal count */
      THREAD_SETMEM(self, p_signal, 0);
      /* Say we're in sigwait */
      THREAD_SETMEM(self, p_sigwaiting, 1);
      /* Unblock the signals and wait for them */
      sigsuspend(&mask);
    }
  }
  THREAD_SETMEM(self, p_cancel_jmp, NULL);
  /* The signals are now reblocked.  Check for cancellation */
  pthread_testcancel();
  /* We should have self->p_signal != 0 and equal to the signal received */
  *sig = THREAD_GETMEM(self, p_signal);
  return 0;
}

/* Redefine raise() to send signal to calling thread only,
   as per POSIX 1003.1c */
int raise (int sig)
{
  int retcode = pthread_kill(pthread_self(), sig);
  if (retcode == 0)
    return 0;
  else {
    errno = retcode;
    return -1;
  }
}
