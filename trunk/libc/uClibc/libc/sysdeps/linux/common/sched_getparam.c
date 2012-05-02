/* vi: set sw=4 ts=4: */
/*
 * sched_getparam() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sched.h>

#define __NR___syscall_sched_getparam __NR_sched_getparam
static inline _syscall2(int, __syscall_sched_getparam,
		__kernel_pid_t, pid, struct sched_param *, p);

int sched_getparam(pid_t pid, struct sched_param *p)
{
	return (__syscall_sched_getparam(pid, p));
}
