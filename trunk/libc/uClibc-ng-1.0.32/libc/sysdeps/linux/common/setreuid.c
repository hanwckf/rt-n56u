/*
 * setreuid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <bits/wordsize.h>


#if (__WORDSIZE == 32 && defined(__NR_setreuid32)) || __WORDSIZE == 64
# ifdef __NR_setreuid32
#  undef __NR_setreuid
#  define __NR_setreuid __NR_setreuid32
# endif

_syscall2(int, setreuid, uid_t, ruid, uid_t, euid)

#else

# define __NR___syscall_setreuid __NR_setreuid
static __inline__ _syscall2(int, __syscall_setreuid,
		__kernel_uid_t, ruid, __kernel_uid_t, euid)

int setreuid(uid_t ruid, uid_t euid)
{
	if (((ruid + 1) > (uid_t) ((__kernel_uid_t) - 1U))
		|| ((euid + 1) > (uid_t) ((__kernel_uid_t) - 1U))) {
		__set_errno(EINVAL);
		return -1;
	}
	return (__syscall_setreuid(ruid, euid));
}
#endif

libc_hidden_weak(setreuid)
