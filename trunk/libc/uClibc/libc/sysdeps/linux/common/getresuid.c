/* vi: set sw=4 ts=4: */
/*
 * getresuid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"

#ifdef __NR_getresuid
#define __NR___syscall_getresuid __NR_getresuid
static inline _syscall3(int, __syscall_getresuid, __kernel_uid_t *, ruid,
		  __kernel_uid_t *, euid, __kernel_uid_t *, suid);

int getresuid(uid_t * ruid, uid_t * euid, uid_t * suid)
{
	int result;
	__kernel_uid_t k_ruid, k_euid, k_suid;

	result = __syscall_getresuid(&k_ruid, &k_euid, &k_suid);
	if (result == 0) {
		*ruid = (uid_t) k_ruid;
		*euid = (uid_t) k_euid;
		*suid = (uid_t) k_suid;
	}
	return result;
}
#endif
