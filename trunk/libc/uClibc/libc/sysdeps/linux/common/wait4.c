/* vi: set sw=4 ts=4: */
/*
 * wait4() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"

#define __NR___syscall_wait4 __NR_wait4
static inline _syscall4(int, __syscall_wait4, __kernel_pid_t, pid,
		int *, status, int, opts, void *, rusage);

int wait4(pid_t pid, int *status, int opts, void *rusage)
{
	return (__syscall_wait4(pid, status, opts, rusage));
}
