/* vi: set sw=4 ts=4: */
/*
 * setregid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#define __NR___syscall_setregid __NR_setregid
static inline _syscall2(int, __syscall_setregid,
		__kernel_gid_t, rgid, __kernel_gid_t, egid);

int setregid(gid_t rgid, gid_t egid)
{
	if (((rgid + 1) > (gid_t) ((__kernel_gid_t) - 1U))
		|| ((egid + 1) > (gid_t) ((__kernel_gid_t) - 1U))) {
		__set_errno(EINVAL);
		return -1;
	}
	return (__syscall_setregid(rgid, egid));
}
