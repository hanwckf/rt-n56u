/*
 * stat64() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <sys/stat.h>

#if defined __NR_fstatat64 && !defined __NR_stat64
#include <fcntl.h>
#include <unistd.h>

int stat64(const char *file_name, struct stat64 *buf)
{
	return fstatat64(AT_FDCWD, file_name, buf, 0);
}
libc_hidden_def(stat64)

/* For systems which have both, prefer the old one */
# elif defined __NR_stat64
# define __NR___syscall_stat64 __NR_stat64
# include "xstatconv.h"
static __always_inline _syscall2(int, __syscall_stat64,
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
