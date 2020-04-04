/*
 * Copyright (C) 2006 Steven J. Hill <sjhill@realitydiluted.com>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/wait.h>
#include <cancel.h>

pid_t __NC(waitpid)(pid_t pid, int *wait_stat, int options)
{
#if 1 /* kernel/exit.c says to avoid waitpid syscall */
	return __wait4_nocancel(pid, wait_stat, options, NULL);
#else
	return INLINE_SYSCALL(waitpid, 3, pid, wait_stat, options);
#endif
}
CANCELLABLE_SYSCALL(pid_t, waitpid, (pid_t pid, int *wait_stat, int options), (pid, wait_stat, options))
lt_libc_hidden(waitpid)
