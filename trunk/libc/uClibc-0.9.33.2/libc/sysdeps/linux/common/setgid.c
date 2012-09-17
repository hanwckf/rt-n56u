/* vi: set sw=4 ts=4: */
/*
 * setgid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <bits/wordsize.h>

#if (__WORDSIZE == 32 && defined(__NR_setgid32)) || __WORDSIZE == 64
# ifdef __NR_setgid32
#  undef __NR_setgid
#  define __NR_setgid __NR_setgid32
# endif

_syscall1(int, setgid, gid_t, gid)

#else

# define __NR___syscall_setgid __NR_setgid
static __inline__ _syscall1(int, __syscall_setgid, __kernel_gid_t, gid)

int setgid(gid_t gid)
{
	if (gid == (gid_t) ~ 0 || gid != (gid_t) ((__kernel_gid_t) gid)) {
		__set_errno(EINVAL);
		return -1;
	}
	return (__syscall_setgid(gid));
}
#endif
