/*
 * pause() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <cancel.h>

#ifdef __NR_pause
/* even if it is not obvious, glibc uses the pause syscall, see syscalls.list */
# define __NR___pause_nocancel __NR_pause
static _syscall0(int, __NC(pause))
CANCELLABLE_SYSCALL(int, pause, (void), ())
#else
# define __need_NULL
# include <stddef.h>
# include <signal.h>

int
# ifdef __UCLIBC_HAS_LINUXTHREADS__
weak_function
# endif
__NC(pause)(void)
{
	sigset_t set;

	/*__sigemptyset (&set); - why? */
	sigprocmask (SIG_BLOCK, NULL, &set);

	/* pause is a cancellation point, but so is sigsuspend.
	   So no need for anything special here.  */
	return sigsuspend(&set);
}
CANCELLABLE_SYSCALL(int, pause, (void), ())
LIBC_CANCEL_HANDLED ();		/* sigsuspend handles our cancellation.  */
#endif
