/* vi: set sw=4 ts=4: */
/*
 * sched_rr_get_interval() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define __NR___syscall_sched_rr_get_interval __NR_sched_rr_get_interval
static __inline__ _syscall2(int, __syscall_sched_rr_get_interval,
		__kernel_pid_t, pid, struct timespec *, tp)

int sched_rr_get_interval(pid_t pid, struct timespec *tp)
{
	return (__syscall_sched_rr_get_interval(pid, tp));
}
