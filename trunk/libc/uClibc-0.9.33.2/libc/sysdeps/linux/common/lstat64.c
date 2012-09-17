/* vi: set sw=4 ts=4: */
/*
 * lstat64() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __UCLIBC_HAS_LFS__ && defined __NR_lstat64
# include <unistd.h>
# include <sys/stat.h>
# include "xstatconv.h"


# define __NR___syscall_lstat64 __NR_lstat64
static __inline__ _syscall2(int, __syscall_lstat64, const char *, file_name,
		  struct kernel_stat64 *, buf)

int lstat64(const char *file_name, struct stat64 *buf)
{
	int result;
	struct kernel_stat64 kbuf;

	result = __syscall_lstat64(file_name, &kbuf);
	if (result == 0) {
		__xstat64_conv(&kbuf, buf);
	}
	return result;
}
libc_hidden_def(lstat64)

#endif
