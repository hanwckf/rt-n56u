/* Copyright (C) 1998, 2000 Free Software Foundation, Inc.
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
#define __need_NULL
#include <stddef.h>
#include <signal.h>


/* Set the disposition for SIG.  */
__sighandler_t sigset (int sig, __sighandler_t disp)
{
    struct sigaction act, oact;
    sigset_t set;

#ifdef SIG_HOLD
    /* Handle SIG_HOLD first.  */
    if (disp == SIG_HOLD)
    {
	/* Create an empty signal set.  */
	if (__sigemptyset (&set) < 0)
	    return SIG_ERR;

	/* Add the specified signal.  */
	if (__sigaddset (&set, sig) < 0)
	    return SIG_ERR;

	/* Add the signal set to the current signal mask.  */
	if (sigprocmask (SIG_BLOCK, &set, NULL) < 0)
	    return SIG_ERR;

	return SIG_HOLD;
    }
#endif	/* SIG_HOLD */

    /* Check signal extents to protect __sigismember.  */
    if (disp == SIG_ERR || sig < 1 || sig >= NSIG)
    {
	__set_errno (EINVAL);
	return SIG_ERR;
    }

    act.sa_handler = disp;
    if (__sigemptyset (&act.sa_mask) < 0)
	return SIG_ERR;
    act.sa_flags = 0;
    if (sigaction (sig, &act, &oact) < 0)
	return SIG_ERR;

    /* Create an empty signal set.  */
    if (__sigemptyset (&set) < 0)
	return SIG_ERR;

    /* Add the specified signal.  */
    if (__sigaddset (&set, sig) < 0)
	return SIG_ERR;

    /* Remove the signal set from the current signal mask.  */
    if (sigprocmask (SIG_UNBLOCK, &set, NULL) < 0)
	return SIG_ERR;

    return oact.sa_handler;
}
