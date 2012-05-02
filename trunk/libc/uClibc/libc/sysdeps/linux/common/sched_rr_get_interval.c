/* vi: set sw=4 ts=4: */
/*
 * sched_rr_get_interval() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sched.h>

#define __NR___syscall_sched_rr_get_interval __NR_sched_rr_get_interval
static inline _syscall2(int, __syscall_sched_rr_get_interval,
		__kernel_pid_t, pid, struct timespec *, tp);

int sched_rr_get_interval(pid_t pid, struct timespec *tp)
{
	return (__syscall_sched_rr_get_interval(pid, tp));
}
