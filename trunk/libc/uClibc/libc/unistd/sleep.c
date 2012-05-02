/* Implementation of the POSIX sleep function using nanosleep.
   Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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
#include <time.h>
#include <signal.h>
#include <unistd.h>

#if 0
/* This is a quick and dirty, but not 100% compliant with
 * the stupid SysV SIGCHLD vs. SIG_IGN behaviour.  It is
 * fine unless you are messing with SIGCHLD...  */
unsigned int sleep (unsigned int sec)
{
	unsigned int res;
	struct timespec ts = { .tv_sec = (long int) seconds, .tv_nsec = 0 };
	res = nanosleep(&ts, &ts);
	if (res) res = (unsigned int) ts.tv_sec + (ts.tv_nsec >= 500000000L);
	return res;
}

#else

/* We are going to use the `nanosleep' syscall of the kernel.  But the
   kernel does not implement the sstupid SysV SIGCHLD vs. SIG_IGN
   behaviour for this syscall.  Therefore we have to emulate it here.  */
unsigned int sleep (unsigned int seconds)
{
    struct timespec ts = { .tv_sec = (long int) seconds, .tv_nsec = 0 };
    sigset_t set, oset;
    unsigned int result;

    /* This is not necessary but some buggy programs depend on this.  */
    if (seconds == 0)
	return 0;

    /* Linux will wake up the system call, nanosleep, when SIGCHLD
       arrives even if SIGCHLD is ignored.  We have to deal with it
       in libc.  We block SIGCHLD first.  */
    if (__sigemptyset (&set) < 0
	    || __sigaddset (&set, SIGCHLD) < 0
	    || sigprocmask (SIG_BLOCK, &set, &oset))
	return -1;

    /* If SIGCHLD is already blocked, we don't have to do anything.  */
    if (!__sigismember (&oset, SIGCHLD))
    {
	int saved_errno;
	struct sigaction oact;

	if (__sigemptyset (&set) < 0 || __sigaddset (&set, SIGCHLD) < 0)
	    return -1;

	/* We get the signal handler for SIGCHLD.  */
	if (sigaction (SIGCHLD, (struct sigaction *) NULL, &oact) < 0)
	{
	    saved_errno = errno;
	    /* Restore the original signal mask.  */
	    (void) sigprocmask (SIG_SETMASK, &oset, (sigset_t *) NULL);
	    __set_errno (saved_errno);
	    return -1;
	}

	if (oact.sa_handler == SIG_IGN)
	{
	    /* We should leave SIGCHLD blocked.  */
	    result = nanosleep (&ts, &ts);

	    saved_errno = errno;
	    /* Restore the original signal mask.  */
	    (void) sigprocmask (SIG_SETMASK, &oset, (sigset_t *) NULL);
	    __set_errno (saved_errno);
	}
	else
	{
	    /* We should unblock SIGCHLD.  Restore the original signal mask.  */
	    (void) sigprocmask (SIG_SETMASK, &oset, (sigset_t *) NULL);
	    result = nanosleep (&ts, &ts);
	}
    }
    else
	result = nanosleep (&ts, &ts);

    if (result != 0)
	/* Round remaining time.  */
	result = (unsigned int) ts.tv_sec + (ts.tv_nsec >= 500000000L);

    return result;
}
#endif
