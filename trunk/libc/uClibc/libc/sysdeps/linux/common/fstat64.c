/* vi: set sw=4 ts=4: */
/*
 * fstat64() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"

#if defined __UCLIBC_HAS_LFS__ && defined __NR_fstat64
#define __NR___syscall_fstat64 __NR_fstat64
#include <unistd.h>
#include <sys/stat.h>
#include <bits/kernel_stat.h>
#include "xstatconv.h"

static inline _syscall2(int, __syscall_fstat64,
		int, filedes, struct kernel_stat64 *, buf);

int fstat64(int fd, struct stat64 *buf)
{
	int result;
	struct kernel_stat64 kbuf;

	result = __syscall_fstat64(fd, &kbuf);
	if (result == 0) {
		__xstat64_conv(&kbuf, buf);
	}
	return result;
}
#endif							/* __UCLIBC_HAS_LFS__ */
