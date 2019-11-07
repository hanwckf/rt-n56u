/*
 * kill() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <signal.h>


#define __NR___syscall_kill __NR_kill
static __inline__ _syscall2(int, __syscall_kill, __kernel_pid_t, pid, int, sig)

int kill(pid_t pid, int sig)
{
	return (__syscall_kill(pid, sig));
}
libc_hidden_def(kill)
