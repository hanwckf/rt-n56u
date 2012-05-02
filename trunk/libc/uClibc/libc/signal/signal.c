/* BSD-like signal function.
   Copyright (C) 1991, 1992, 1996, 1997, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <signal.h>


sigset_t _sigintr;		/* Set by siginterrupt.  */

/* Set the handler for the signal SIG to HANDLER,
   returning the old handler, or SIG_ERR on error.  */
__sighandler_t bsd_signal (int sig, __sighandler_t handler)
{
    struct sigaction act, oact;

    /* Check signal extents to protect __sigismember.  */
    if (handler == SIG_ERR || sig < 1 || sig >= NSIG)
    {
	__set_errno (EINVAL);
	return SIG_ERR;
    }

    act.sa_handler = handler;
    if (__sigemptyset (&act.sa_mask) < 0
	    || __sigaddset (&act.sa_mask, sig) < 0)
	return SIG_ERR;
    act.sa_flags = __sigismember (&_sigintr, sig) ? 0 : SA_RESTART;
    if (sigaction (sig, &act, &oact) < 0)
	return SIG_ERR;

    return oact.sa_handler;
}
weak_alias (bsd_signal, signal)
