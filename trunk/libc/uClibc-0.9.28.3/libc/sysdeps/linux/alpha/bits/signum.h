/* Signal number definitions.  Linux/Alpha version.
   Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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

#ifdef	_SIGNAL_H

/* Fake signal functions.  */
#define SIG_ERR ((__sighandler_t) -1) /* Error return.  */
#define SIG_DFL ((__sighandler_t) 0) /* Default action.  */
#define SIG_IGN ((__sighandler_t) 1) /* Ignore signal.  */

#ifdef __USE_UNIX98
# define SIG_HOLD	((__sighandler_t) 2)	/* Add signal to hold mask.  */
#endif

/*
 * Linux/AXP has different signal numbers that Linux/i386: I'm trying
 * to make it OSF/1 binary compatible, at least for normal binaries.
 */
#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGEMT		 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGBUS		10
#define SIGSEGV		11
#define SIGSYS		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGURG		16
#define SIGSTOP		17
#define SIGTSTP		18
#define SIGCONT		19
#define SIGCHLD		20
#define SIGCLD          SIGCHLD
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGIO		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGINFO		29
#define SIGUSR1		30
#define SIGUSR2		31

#define SIGPOLL	SIGIO
#define SIGPWR	SIGINFO
#define SIGIOT	SIGABRT

#define	_NSIG		64	/* Biggest signal number + 1.  */

#define SIGRTMIN	(__libc_current_sigrtmin ())
#define SIGRTMAX	(__libc_current_sigrtmax ())

/* These are the hard limits of the kernel.  These values should not be
   used directly at user level.  */
#define __SIGRTMIN	32
#define __SIGRTMAX	(_NSIG - 1)

#endif	/* <signal.h> included.  */
