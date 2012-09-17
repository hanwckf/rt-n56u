/* vi: set sw=4 ts=4: */
/*
 * fstat64() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __UCLIBC_HAS_LFS__ && defined __NR_fstat64
#include <unistd.h>
#include <sys/stat.h>
#include "xstatconv.h"


#define __NR___syscall_fstat64 __NR_fstat64
static __inline__ _syscall2(int, __syscall_fstat64,
		int, filedes, struct kernel_stat64 *, buf)

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
libc_hidden_def(fstat64)
#endif
