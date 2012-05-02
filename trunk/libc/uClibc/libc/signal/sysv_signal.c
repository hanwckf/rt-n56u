/* Copyright (C) 1991, 1992, 1996, 1997 Free Software Foundation, Inc.
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

/* Tolerate non-threads versions of Posix */
#ifndef SA_ONESHOT
#define SA_ONESHOT 0
#endif
#ifndef SA_NOMASK
#define SA_NOMASK 0
#endif
#ifndef SA_INTERRUPT
#define SA_INTERRUPT 0
#endif

/* Set the handler for the signal SIG to HANDLER,
   returning the old handler, or SIG_ERR on error.  */
__sighandler_t __sysv_signal (int sig, __sighandler_t handler)
{
    struct sigaction act, oact;

    /* Check signal extents to protect __sigismember.  */
    if (handler == SIG_ERR || sig < 1 || sig >= NSIG)
    {
	__set_errno (EINVAL);
	return SIG_ERR;
    }

    act.sa_handler = handler;
    if (__sigemptyset (&act.sa_mask) < 0)
	return SIG_ERR;
    act.sa_flags = SA_ONESHOT | SA_NOMASK | SA_INTERRUPT;
    act.sa_flags &= ~SA_RESTART;
    if (sigaction (sig, &act, &oact) < 0)
	return SIG_ERR;

    return oact.sa_handler;
}
weak_alias (__sysv_signal, sysv_signal)
