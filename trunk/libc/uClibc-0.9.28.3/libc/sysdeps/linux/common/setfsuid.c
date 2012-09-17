/* vi: set sw=4 ts=4: */
/*
 * setfsuid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/fsuid.h>

#define __NR___syscall_setfsuid __NR_setfsuid
static inline _syscall1(int, __syscall_setfsuid, __kernel_uid_t, uid);

int setfsuid(uid_t uid)
{
	if (uid != (uid_t) ((__kernel_uid_t) uid)) {
		__set_errno(EINVAL);
		return -1;
	}
	return (__syscall_setfsuid(uid));
}
