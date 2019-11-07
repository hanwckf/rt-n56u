/*
 * getpgid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __USE_UNIX98
#include <unistd.h>

#define __NR___syscall_getpgid __NR_getpgid
static __inline__ _syscall1(__kernel_pid_t, __syscall_getpgid, __kernel_pid_t, pid)

pid_t __getpgid(pid_t pid)
{
	return (__syscall_getpgid(pid));
}
#ifdef __USE_XOPEN_EXTENDED
weak_alias(__getpgid,getpgid)
#endif
#endif
