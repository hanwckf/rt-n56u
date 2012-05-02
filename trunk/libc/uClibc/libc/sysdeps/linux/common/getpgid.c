/* vi: set sw=4 ts=4: */
/*
 * getpgid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"

#define __NR___syscall_getpgid __NR_getpgid
static inline _syscall1(__kernel_pid_t, __syscall_getpgid, __kernel_pid_t, pid);

pid_t __getpgid(pid_t pid)
{
	return (__syscall_getpgid(pid));
}
weak_alias(__getpgid, getpgid);
