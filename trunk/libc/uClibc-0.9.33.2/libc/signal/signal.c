/* BSD-like signal function.
   Copyright (C) 1991,1992,1996,1997,2000,2002,2005
   Free Software Foundation, Inc.
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
#include <signal.h>
#include <string.h>	/* For the real memset prototype.  */

sigset_t _sigintr attribute_hidden;		/* Set by siginterrupt.  */

/* Set the handler for the signal SIG to HANDLER,
   returning the old handler, or SIG_ERR on error.  */
__sighandler_t
signal (int sig, __sighandler_t handler)
{
  struct sigaction act, oact;

  /* Check signal extents to protect __sigismember.  */
  if (handler == SIG_ERR || sig < 1 || sig >= NSIG)
    {
      __set_errno (EINVAL);
      return SIG_ERR;
    }

  act.sa_handler = handler;
  __sigemptyset (&act.sa_mask);
  __sigaddset (&act.sa_mask, sig);
  act.sa_flags = __sigismember (&_sigintr, sig) ? 0 : SA_RESTART;
  /* In Linux (as of 2.6.25), fails only if sig is SIGKILL or SIGSTOP */
  if (sigaction (sig, &act, &oact) < 0)
    return SIG_ERR;

  return oact.sa_handler;
}
libc_hidden_def(signal)
#ifdef __UCLIBC_SUSV3_LEGACY__
strong_alias(signal,bsd_signal)
#endif
