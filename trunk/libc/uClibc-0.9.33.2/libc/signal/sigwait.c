/* vi: set sw=4 ts=4: */
/* sigwait
 *
 * Copyright (C) 2006 by Steven J. Hill <sjhill@realitydiluted.com>
 * Copyright (C) 2003-2005 by Erik Andersen <andersen@uclibc.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The GNU C Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the GNU C Library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA.  */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
# include <sysdep-cancel.h>

# ifdef __NR_rt_sigtimedwait

/* Return any pending signal or wait for one for the given time.  */
static int do_sigwait(const sigset_t *set, int *sig)
{
	int ret;

#  ifdef SIGCANCEL
	sigset_t tmpset;
	if (set != NULL
		&& (__builtin_expect (__sigismember (set, SIGCANCEL), 0)
#   ifdef SIGSETXID
		|| __builtin_expect (__sigismember (set, SIGSETXID), 0)
#   endif
		))
	{
		/* Create a temporary mask without the bit for SIGCANCEL set.  */
		// We are not copying more than we have to.
		memcpy(&tmpset, set, _NSIG / 8);
		__sigdelset(&tmpset, SIGCANCEL);
#   ifdef SIGSETXID
		__sigdelset(&tmpset, SIGSETXID);
#   endif
		set = &tmpset;
	}
#  endif

	/* XXX The size argument hopefully will have to be changed to the
	   real size of the user-level sigset_t.  */
	INTERNAL_SYSCALL_DECL(err);
	do
		ret = INTERNAL_SYSCALL (rt_sigtimedwait, err, 4, set, NULL,
			NULL, _NSIG / 8);
	while (INTERNAL_SYSCALL_ERROR_P (ret, err)
		&& INTERNAL_SYSCALL_ERRNO (ret, err) == EINTR);
	if (! INTERNAL_SYSCALL_ERROR_P (ret, err))
	{
		*sig = ret;
		ret = 0;
	}
else
	ret = INTERNAL_SYSCALL_ERRNO (ret, err);

	return ret;
}

int sigwait (const sigset_t *set, int *sig)
{
	if(SINGLE_THREAD_P)
		return do_sigwait(set, sig);

	int oldtype = LIBC_CANCEL_ASYNC();

	int result = do_sigwait(set, sig);

	LIBC_CANCEL_RESET(oldtype);

	return result;
}
# else /* __NR_rt_sigtimedwait */
#  error We must have rt_sigtimedwait defined!!!
# endif
#else /* __UCLIBC_HAS_THREADS_NATIVE__ */

# if defined __UCLIBC_HAS_REALTIME__

int sigwait (const sigset_t *set, int *sig)
{
	int ret = 1;
	if ((ret = sigwaitinfo(set, NULL)) != -1) {
		*sig = ret;
		return 0;
	}
	return 1;
}

# else /* __UCLIBC_HAS_REALTIME__ */
/* variant without REALTIME extensions */

static smallint was_sig; /* obviously not thread-safe */

static void ignore_signal(int sig)
{
	was_sig = sig;
}

int sigwait (const sigset_t *set, int *sig)
{
  sigset_t tmp_mask;
  struct sigaction saved[NSIG];
  struct sigaction action;
  int save_errno;
  int this;

  /* Prepare set.  */
  __sigfillset (&tmp_mask);

  /* Unblock all signals in the SET and register our nice handler.  */
  action.sa_handler = ignore_signal;
  action.sa_flags = 0;
  __sigfillset (&action.sa_mask);       /* Block all signals for handler.  */

  /* Make sure we recognize error conditions by setting WAS_SIG to a
     value which does not describe a legal signal number.  */
  was_sig = -1;

  for (this = 1; this < NSIG; ++this)
    if (__sigismember (set, this))
      {
        /* Unblock this signal.  */
        __sigdelset (&tmp_mask, this);

        /* Register temporary action handler.  */
        /* In Linux (as of 2.6.25), fails only if sig is SIGKILL or SIGSTOP */
        /* (so, will it work correctly if set has, say, SIGSTOP?) */
        if (sigaction (this, &action, &saved[this]) != 0)
          goto restore_handler;
      }

  /* Now we can wait for signals.  */
  sigsuspend (&tmp_mask);

 restore_handler:
  save_errno = errno;

  while (--this >= 1)
    if (__sigismember (set, this))
      /* We ignore errors here since we must restore all handlers.  */
      sigaction (this, &saved[this], NULL);

  __set_errno (save_errno);

  /* Store the result and return.  */
  *sig = was_sig;
  return was_sig == -1 ? -1 : 0;
}
# endif /* __UCLIBC_HAS_REALTIME__ */
#endif /* __UCLIBC_HAS_THREADS_NATIVE__ */
