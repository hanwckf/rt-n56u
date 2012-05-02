/* vi: set sw=4 ts=4: */
/*
 * sched_getscheduler() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sched.h>

#define __NR___syscall_sched_getscheduler __NR_sched_getscheduler
static inline _syscall1(int, __syscall_sched_getscheduler, __kernel_pid_t, pid);

int sched_getscheduler(pid_t pid)
{
	return (__syscall_sched_getscheduler(pid));
}
