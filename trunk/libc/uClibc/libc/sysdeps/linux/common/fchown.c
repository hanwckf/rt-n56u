/* vi: set sw=4 ts=4: */
/*
 * fchown() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#define __NR___syscall_fchown __NR_fchown
static inline _syscall3(int, __syscall_fchown, int, fd,
		__kernel_uid_t, owner, __kernel_gid_t, group);

int fchown(int fd, uid_t owner, gid_t group)
{
	if (((owner + 1) > (uid_t) ((__kernel_uid_t) - 1U))
		|| ((group + 1) > (gid_t) ((__kernel_gid_t) - 1U))) {
		__set_errno(EINVAL);
		return -1;
	}
	return (__syscall_fchown(fd, owner, group));
}
