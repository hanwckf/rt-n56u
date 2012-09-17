/* vi: set sw=4 ts=4: */
/*
 * setresuid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"

#ifdef __NR_setresuid
#define __NR___syscall_setresuid __NR_setresuid
static inline _syscall3(int, __syscall_setresuid,
		__kernel_uid_t, rgid, __kernel_uid_t, egid, __kernel_uid_t, sgid);

int setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
	if (((ruid + 1) > (uid_t) ((__kernel_uid_t) - 1U))
		|| ((euid + 1) > (uid_t) ((__kernel_uid_t) - 1U))
		|| ((suid + 1) > (uid_t) ((__kernel_uid_t) - 1U))) {
		__set_errno(EINVAL);
		return -1;
	}
	return (__syscall_setresuid(ruid, euid, suid));
}
#endif
