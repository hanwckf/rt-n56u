/* vi: set sw=4 ts=4: */
/*
 * getresgid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#ifdef __USE_GNU
#include <unistd.h>

#if defined(__NR_getresgid32)
# undef __NR_getresgid
# define __NR_getresgid __NR_getresgid32
_syscall3(int, getresgid, gid_t *, rgid, gid_t *, egid, gid_t *, sgid)

#elif defined(__NR_getresgid)
# define __NR___syscall_getresgid __NR_getresgid
static __inline__ _syscall3(int, __syscall_getresgid, __kernel_gid_t *, rgid,
		  __kernel_gid_t *, egid, __kernel_gid_t *, sgid)

int getresgid(gid_t * rgid, gid_t * egid, gid_t * sgid)
{
	int result;
	__kernel_gid_t k_rgid, k_egid, k_sgid;

	result = __syscall_getresgid(&k_rgid, &k_egid, &k_sgid);
	if (result == 0) {
		*rgid = (gid_t) k_rgid;
		*egid = (gid_t) k_egid;
		*sgid = (gid_t) k_sgid;
	}
	return result;
}
#endif
#endif
