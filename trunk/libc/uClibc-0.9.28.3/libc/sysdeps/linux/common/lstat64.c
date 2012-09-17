/* vi: set sw=4 ts=4: */
/*
 * lstat64() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"

#if defined __UCLIBC_HAS_LFS__ && defined __NR_lstat64
#include <unistd.h>
#include <sys/stat.h>
#include <bits/kernel_stat.h>
#include "xstatconv.h"

#define __NR___syscall_lstat64 __NR_lstat64
static inline _syscall2(int, __syscall_lstat64, const char *, file_name,
		  struct kernel_stat64 *, buf);

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
#endif							/* __UCLIBC_HAS_LFS__ */
