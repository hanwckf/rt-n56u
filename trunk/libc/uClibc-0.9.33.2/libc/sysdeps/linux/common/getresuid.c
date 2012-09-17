/* vi: set sw=4 ts=4: */
/*
 * getresuid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#ifdef __USE_GNU
#include <unistd.h>

#if defined(__NR_getresuid32)
# undef __NR_getresuid
# define __NR_getresuid __NR_getresuid32
_syscall3(int, getresuid, uid_t *, ruid, uid_t *, euid, uid_t *, suid)

#elif defined(__NR_getresuid)
# define __NR___syscall_getresuid __NR_getresuid
static __inline__ _syscall3(int, __syscall_getresuid, __kernel_uid_t *, ruid,
		  __kernel_uid_t *, euid, __kernel_uid_t *, suid)

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
#endif
