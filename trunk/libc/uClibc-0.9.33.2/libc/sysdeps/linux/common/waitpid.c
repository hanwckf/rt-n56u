/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2006 Steven J. Hill <sjhill@realitydiluted.com>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include "sysdep-cancel.h"
#else
#define SINGLE_THREAD_P 1
#endif

libc_hidden_proto(wait4)

extern __typeof(waitpid) __libc_waitpid;
__pid_t __libc_waitpid(__pid_t pid, int *wait_stat, int options)
{
	if (SINGLE_THREAD_P)
		return wait4(pid, wait_stat, options, NULL);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = wait4(pid, wait_stat, options, NULL);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
libc_hidden_proto(waitpid)
weak_alias(__libc_waitpid,waitpid)
libc_hidden_weak(waitpid)
