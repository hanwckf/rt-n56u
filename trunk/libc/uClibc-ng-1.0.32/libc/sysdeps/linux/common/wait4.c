/*
 * wait4() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/wait.h>

# define __NR___syscall_wait4 __NR_wait4
static __always_inline _syscall4(int, __syscall_wait4, __kernel_pid_t, pid,
				 int *, status, int, opts, struct rusage *, rusage)

pid_t __wait4_nocancel(pid_t pid, int *status, int opts, struct rusage *rusage)
{
	return __syscall_wait4(pid, status, opts, rusage);
}
#ifdef __USE_BSD
strong_alias(__wait4_nocancel,wait4)
#endif
