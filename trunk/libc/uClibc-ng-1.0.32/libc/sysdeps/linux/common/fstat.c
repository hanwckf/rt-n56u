/*
 * fstat() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "xstatconv.h"

#if defined __NR_fstat64 && !defined __NR_fstat
int fstat(int fd, struct stat *buf)
{
	return INLINE_SYSCALL(fstat64, 2, fd, buf);
}
libc_hidden_def(fstat)

#elif __WORDSIZE == 64 && defined __NR_newfstatat && !defined __ARCH_HAS_DEPRECATED_SYSCALLS__
#include <fcntl.h>

int fstat(int fd, struct stat *buf)
{
	return INLINE_SYSCALL(fstat, 2, fd, buf);
}
libc_hidden_def(fstat)

#elif defined __NR_fstat
int fstat(int fd, struct stat *buf)
{
	int result;
# ifdef __NR_fstat64
	/* normal stat call has limited values for various stat elements
	 * e.g. uid device major/minor etc.
	 * so we use 64 variant if available
	 * in order to get newer versions of stat elements
	 */
	struct kernel_stat64 kbuf;
	result = INLINE_SYSCALL(fstat64, 2, fd, &kbuf);
	if (result == 0) {
		__xstat32_conv(&kbuf, buf);
	}
# else
	struct kernel_stat kbuf;

	result = INLINE_SYSCALL(fstat, 2, fd, &kbuf);
	if (result == 0) {
		__xstat_conv(&kbuf, buf);
	}
# endif
	return result;
}
libc_hidden_def(fstat)
#endif

# if ! defined __NR_fstat64
strong_alias_untyped(fstat,fstat64)
libc_hidden_def(fstat64)
#endif
