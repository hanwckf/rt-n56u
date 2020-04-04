/*
 * Copyright (C) 2006 Steven J. Hill <sjhill@realitydiluted.com>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
#include <sys/wait.h>
#include <cancel.h>

static pid_t __NC(wait)(__WAIT_STATUS_DEFN stat_loc)
{
	return __wait4_nocancel(WAIT_ANY, stat_loc, 0, (struct rusage *)NULL);
}
CANCELLABLE_SYSCALL(pid_t, wait, (__WAIT_STATUS_DEFN stat_loc), (stat_loc))
