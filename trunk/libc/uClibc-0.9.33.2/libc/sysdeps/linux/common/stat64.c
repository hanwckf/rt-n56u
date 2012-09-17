/* vi: set sw=4 ts=4: */
/*
 * stat64() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>

#if defined __UCLIBC_HAS_LFS__ && defined __NR_stat64

# define __NR___syscall_stat64 __NR_stat64
# include <unistd.h>
# include "xstatconv.h"

static __inline__ _syscall2(int, __syscall_stat64,
		const char *, file_name, struct kernel_stat64 *, buf)

int stat64(const char *file_name, struct stat64 *buf)
{
	int result;
	struct kernel_stat64 kbuf;

	result = __syscall_stat64(file_name, &kbuf);
	if (result == 0) {
		__xstat64_conv(&kbuf, buf);
	}
	return result;
}
libc_hidden_def(stat64)
#endif
