/* vi: set sw=4 ts=4: */
/*
 * wait4() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
#include <sys/wait.h>
#include <sys/resource.h>


#define __NR___syscall_wait4 __NR_wait4
static __inline__ _syscall4(int, __syscall_wait4, __kernel_pid_t, pid,
		int *, status, int, opts, struct rusage *, rusage)

pid_t wait4(pid_t pid, int *status, int opts, struct rusage *rusage)
{
	return (__syscall_wait4(pid, status, opts, rusage));
}
libc_hidden_def(wait4)
#endif
