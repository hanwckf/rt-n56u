/* vi: set sw=4 ts=4: */
/*
 * setregid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <bits/wordsize.h>


#if (__WORDSIZE == 32 && defined(__NR_setregid32)) || __WORDSIZE == 64
# ifdef __NR_setregid32
#  undef __NR_setregid
#  define __NR_setregid __NR_setregid32
# endif

_syscall2(int, setregid, gid_t, rgid, gid_t, egid)

#else

# define __NR___syscall_setregid __NR_setregid
static __inline__ _syscall2(int, __syscall_setregid,
		__kernel_gid_t, rgid, __kernel_gid_t, egid)

int setregid(gid_t rgid, gid_t egid)
{
	if (((rgid + 1) > (gid_t) ((__kernel_gid_t) - 1U))
		|| ((egid + 1) > (gid_t) ((__kernel_gid_t) - 1U))) {
		__set_errno(EINVAL);
		return -1;
	}
	return (__syscall_setregid(rgid, egid));
}
#endif

libc_hidden_def(setregid)
