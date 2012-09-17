/* vi: set sw=4 ts=4: */
/*
 * sched_getscheduler() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define __NR___syscall_sched_getscheduler __NR_sched_getscheduler
static __inline__ _syscall1(int, __syscall_sched_getscheduler, __kernel_pid_t, pid)

int sched_getscheduler(pid_t pid)
{
	return (__syscall_sched_getscheduler(pid));
}
