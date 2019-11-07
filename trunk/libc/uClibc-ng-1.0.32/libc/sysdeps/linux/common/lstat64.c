/*
 * lstat64() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/stat.h>

#if defined __NR_fstatat64 && !defined __NR_lstat
# include <fcntl.h>

int lstat64(const char *file_name, struct stat64 *buf)
{
	return fstatat64(AT_FDCWD, file_name, buf, AT_SYMLINK_NOFOLLOW);
}
libc_hidden_def(lstat64)

#elif __WORDSIZE == 64 && defined __NR_newfstatat
# include <fcntl.h>

int lstat64(const char *file_name, struct stat64 *buf)
{
	return fstatat64(AT_FDCWD, file_name, buf, AT_SYMLINK_NOFOLLOW);
}
libc_hidden_def(lstat64)

/* For systems which have both, prefer the old one */
#elif defined __NR_lstat64
# include "xstatconv.h"
# define __NR___syscall_lstat64 __NR_lstat64
static __always_inline _syscall2(int, __syscall_lstat64, const char *, file_name,
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
