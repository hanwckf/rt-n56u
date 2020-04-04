/*
 * setfsuid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/fsuid.h>
#include <bits/wordsize.h>

#if (__WORDSIZE == 32 && defined(__NR_setfsuid32)) || __WORDSIZE == 64
# ifdef __NR_setfsuid32
#  undef __NR_setfsuid
#  define __NR_setfsuid __NR_setfsuid32
# endif

_syscall1(int, setfsuid, uid_t, uid)

#else

# define __NR___syscall_setfsuid __NR_setfsuid
static __inline__ _syscall1(int, __syscall_setfsuid, __kernel_uid_t, uid)

int setfsuid(uid_t uid)
{
	if (uid != (uid_t) ((__kernel_uid_t) uid)) {
		__set_errno(EINVAL);
		return -1;
	}
	return (__syscall_setfsuid(uid));
}
#endif
