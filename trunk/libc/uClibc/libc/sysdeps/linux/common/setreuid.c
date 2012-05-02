/* vi: set sw=4 ts=4: */
/*
 * setreuid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#define __NR___syscall_setreuid __NR_setreuid
static inline _syscall2(int, __syscall_setreuid,
		__kernel_uid_t, ruid, __kernel_uid_t, euid);

int setreuid(uid_t ruid, uid_t euid)
{
	if (((ruid + 1) > (uid_t) ((__kernel_uid_t) - 1U))
		|| ((euid + 1) > (uid_t) ((__kernel_uid_t) - 1U))) {
		__set_errno(EINVAL);
		return -1;
	}
	return (__syscall_setreuid(ruid, euid));
}
