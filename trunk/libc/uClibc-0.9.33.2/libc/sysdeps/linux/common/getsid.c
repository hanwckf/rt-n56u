/* vi: set sw=4 ts=4: */
/*
 * getsid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __USE_XOPEN_EXTENDED

#define __NR___syscall_getsid __NR_getsid
static __inline__ _syscall1(__kernel_pid_t, __syscall_getsid, __kernel_pid_t, pid)

pid_t getsid(pid_t pid)
{
	return (__syscall_getsid(pid));
}
libc_hidden_def(getsid)
#endif
