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
 * License along with the GNU C Library; see the file COPYING.LIB.  If
 * not, see <http://www.gnu.org/licenses/>.
 */

#define __need_NULL
#include <stddef.h>
#include <sys/syscall.h>
#include <signal.h>
#include <cancel.h>

#if defined __NR_rt_sigtimedwait && defined __UCLIBC_HAS_REALTIME__

#include <string.h>

/* Return any pending signal or wait for one for the given time.  */
static int __NC(sigwait)(const sigset_t *set, int *sig)
{
	int ret;

	do
		/* we might as well use sigtimedwait and do not care about cancellation */
		ret = __NC(sigtimedwait)(set, NULL, NULL);
	while (ret == -1 && errno == EINTR);
	if (ret != -1) {
		*sig = ret;
		ret = 0;
	} else
		ret = errno;

	return ret;
}

#else /* __NR_rt_sigtimedwait */

/* variant without REALTIME extensions */
#include <unistd.h>	/* smallint */

static smallint was_sig; /* obviously not thread-safe */

static void ignore_signal(int sig)
{
	was_sig = sig;
}

static int __NC(sigwait)(const sigset_t *set, int *sig)
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
  __NC(sigsuspend)(&tmp_mask);

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
#endif /* __NR_rt_sigtimedwait */

CANCELLABLE_SYSCALL(int, sigwait, (const sigset_t *set, int *sig), (set, sig))
