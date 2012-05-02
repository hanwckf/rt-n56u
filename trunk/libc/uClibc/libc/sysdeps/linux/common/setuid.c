/* vi: set sw=4 ts=4: */
/*
 * setuid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#define __NR___syscall_setuid __NR_setuid
static inline _syscall1(int, __syscall_setuid, __kernel_uid_t, uid);

int setuid(uid_t uid)
{
	if (uid == (uid_t) ~ 0 || uid != (uid_t) ((__kernel_uid_t) uid)) {
		__set_errno(EINVAL);
		return -1;
	}
	return (__syscall_setuid(uid));
}
