/* Signal number definitions.  Linux/HPPA version.
   Copyright (C) 1995,1996,1997,1998,1999,2003 Free Software Foundation, Inc.
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

#define	SIGHUP		1	/* Hangup (POSIX).  */
#define	SIGINT		2	/* Interrupt (ANSI).  */
#define	SIGQUIT		3	/* Quit (POSIX).  */
#define	SIGILL		4	/* Illegal instruction (ANSI).  */
#define	SIGTRAP		5	/* Trace trap (POSIX).  */
#define	SIGABRT		6	/* Abort (ANSI).  */
#define	SIGIOT		6	/* IOT trap (4.2 BSD).  */
#define	SIGEMT		7
#define	SIGFPE		8	/* Floating-point exception (ANSI).  */
#define	SIGKILL		9	/* Kill, unblockable (POSIX).  */
#define	SIGBUS		10	/* BUS error (4.2 BSD).  */
#define	SIGSEGV		11	/* Segmentation violation (ANSI).  */
#define SIGSYS		12	/* Bad system call.  */
#define	SIGPIPE		13	/* Broken pipe (POSIX).  */
#define	SIGALRM		14	/* Alarm clock (POSIX).  */
#define	SIGTERM		15	/* Termination (ANSI).  */
#define	SIGUSR1		16	/* User-defined signal 1 (POSIX).  */
#define SIGUSR2		17	/* User-defined signal 2 (POSIX).  */
#define	SIGCLD		SIGCHLD	/* Same as SIGCHLD (System V).  */
#define	SIGCHLD		18	/* Child status has changed (POSIX).  */
#define	SIGPWR		19	/* Power failure restart (System V).  */
#define	SIGVTALRM	20	/* Virtual alarm clock (4.2 BSD).  */
#define	SIGPROF		21	/* Profiling alarm clock (4.2 BSD).  */
#define	SIGPOLL		SIGIO	/* Pollable event occurred (System V).  */
#define	SIGIO		22	/* I/O now possible (4.2 BSD).  */
#define	SIGWINCH	23	/* Window size change (4.3 BSD, Sun).  */
#define	SIGSTOP		24	/* Stop, unblockable (POSIX).  */
#define	SIGTSTP		25	/* Keyboard stop (POSIX).  */
#define	SIGCONT		26	/* Continue (POSIX).  */
#define	SIGTTIN		27	/* Background read from tty (POSIX).  */
#define	SIGTTOU		28	/* Background write to tty (POSIX).  */
#define	SIGURG		29	/* Urgent condition on socket (4.2 BSD).  */
#define SIGLOST		30	/* Operating System Has Lost (HP/UX). */
#define SIGUNUSED	31
#define	SIGXCPU		33	/* CPU limit exceeded (4.2 BSD).  */
#define	SIGXFSZ		34	/* File size limit exceeded (4.2 BSD).  */
#define	SIGSTKFLT	36	/* Stack fault.  */

#define __SIGRTMIN	37

#endif	/* <signal.h> included.  */
