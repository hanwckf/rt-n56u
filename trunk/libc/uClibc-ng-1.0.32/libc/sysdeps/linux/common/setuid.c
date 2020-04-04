/*
 * setuid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <bits/wordsize.h>

#if (__WORDSIZE == 32 && defined(__NR_setuid32)) || __WORDSIZE == 64
# ifdef __NR_setuid32
#  undef __NR_setuid
#  define __NR_setuid __NR_setuid32
# endif

_syscall1(int, setuid, uid_t, uid)

#else

# define __NR___syscall_setuid __NR_setuid
static __always_inline _syscall1(int, __syscall_setuid, __kernel_uid_t, uid)

int setuid(uid_t uid)
{
	if (uid == (uid_t) ~ 0 || uid != (uid_t) ((__kernel_uid_t) uid)) {
		__set_errno(EINVAL);
		return -1;
	}
	return __syscall_setuid(uid);
}
#endif
