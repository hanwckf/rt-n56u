/*
 * stat() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <sys/stat.h>

#undef stat

#if defined __NR_fstatat64 && !defined __NR_stat
# include <fcntl.h>

int stat(const char *file_name, struct stat *buf)
{
	return fstatat(AT_FDCWD, file_name, buf, 0);
}

#elif __WORDSIZE == 64 && defined __NR_newfstatat && !defined __ARCH_HAS_DEPRECATED_SYSCALLS__
# include <fcntl.h>

int stat(const char *file_name, struct stat *buf)
{
	return fstatat64(AT_FDCWD, file_name, buf, 0);
}

#else
# include "xstatconv.h"

int stat(const char *file_name, struct stat *buf)
{
	int result;
# ifdef __NR_stat64
	/* normal stat call has limited values for various stat elements
	 * e.g. uid device major/minor etc.
	 * so we use 64 variant if available
	 * in order to get newer versions of stat elements
	 */
	struct kernel_stat64 kbuf;
	result = INLINE_SYSCALL(stat64, 2, file_name, &kbuf);
	if (result == 0) {
		__xstat32_conv(&kbuf, buf);
	}
# else
	struct kernel_stat kbuf;

	result = INLINE_SYSCALL(stat, 2, file_name, &kbuf);
	if (result == 0) {
		__xstat_conv(&kbuf, buf);
	}
# endif /* __NR_stat64 */
	return result;
}
#endif /* __NR_fstat64 */
libc_hidden_def(stat)

#if ! defined __NR_stat64 && ! defined __NR_fstatat64
strong_alias_untyped(stat,stat64)
libc_hidden_def(stat64)
#endif
