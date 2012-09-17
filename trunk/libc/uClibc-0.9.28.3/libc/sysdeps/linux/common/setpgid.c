/* vi: set sw=4 ts=4: */
/*
 * setpgid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#define __NR___syscall_setpgid __NR_setpgid
static inline _syscall2(int, __syscall_setpgid,
		__kernel_pid_t, pid, __kernel_pid_t, pgid);

int setpgid(pid_t pid, pid_t pgid)
{
	return (__syscall_setpgid(pid, pgid));
}
