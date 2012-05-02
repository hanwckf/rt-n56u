/* vi: set sw=4 ts=4: */
/*
 * getsid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#define __NR___syscall_getsid __NR_getsid
static inline _syscall1(__kernel_pid_t, __syscall_getsid, __kernel_pid_t, pid);

pid_t getsid(pid_t pid)
{
	return (__syscall_getsid(pid));
}
