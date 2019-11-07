/*
 * sched_getparam() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define __NR___syscall_sched_getparam __NR_sched_getparam
static __inline__ _syscall2(int, __syscall_sched_getparam,
		__kernel_pid_t, pid, struct sched_param *, p)

int sched_getparam(pid_t pid, struct sched_param *p)
{
	return (__syscall_sched_getparam(pid, p));
}
