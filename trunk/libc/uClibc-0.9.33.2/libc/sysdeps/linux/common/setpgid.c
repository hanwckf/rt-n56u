/* vi: set sw=4 ts=4: */
/*
 * setpgid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __USE_UNIX98 || defined __USE_SVID
#include <unistd.h>


#define __NR___syscall_setpgid __NR_setpgid
static __inline__ _syscall2(int, __syscall_setpgid,
		__kernel_pid_t, pid, __kernel_pid_t, pgid)

int setpgid(pid_t pid, pid_t pgid)
{
	return (__syscall_setpgid(pid, pgid));
}
libc_hidden_def(setpgid)
#endif
