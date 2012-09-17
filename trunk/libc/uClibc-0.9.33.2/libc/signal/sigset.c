/* Copyright (C) 1998, 2000, 2005 Free Software Foundation, Inc.
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
#define __need_NULL
#include <stddef.h>
#include <signal.h>
#include <string.h>	/* For the real memset prototype.  */


/* Set the disposition for SIG.  */
__sighandler_t sigset (int sig, __sighandler_t disp)
{
  struct sigaction act, oact;
  sigset_t set;

  /* Check signal extents to protect __sigismember.  */
  if (disp == SIG_ERR || sig < 1 || sig >= NSIG)
    {
      __set_errno (EINVAL);
      return SIG_ERR;
    }

#ifdef SIG_HOLD
  /* Handle SIG_HOLD first.  */
  if (disp == SIG_HOLD)
    {
      __sigemptyset (&set);
      __sigaddset (&set, sig);

      /* Add the signal set to the current signal mask.  */
      sigprocmask (SIG_BLOCK, &set, NULL); /* can't fail */

      return SIG_HOLD;
    }
#endif	/* SIG_HOLD */

  memset(&act, 0, sizeof(act));
  act.sa_handler = disp;
  /* In Linux (as of 2.6.25), fails only if sig is SIGKILL or SIGSTOP */
  if (sigaction (sig, &act, &oact) < 0)
    return SIG_ERR;

  /* Create an empty signal set. Add the specified signal.  */
  __sigemptyset (&set);
  __sigaddset (&set, sig);

  /* Remove the signal set from the current signal mask.  */
  sigprocmask (SIG_UNBLOCK, &set, NULL); /* can't fail */

  return oact.sa_handler;
}
