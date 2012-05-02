/* vi: set sw=4 ts=4: */
/*
 * lstat() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#define _SYS_STAT_H
#include <bits/stat.h>
#include <bits/kernel_stat.h>
#include "xstatconv.h"

#define __NR___syscall_lstat __NR_lstat
static inline _syscall2(int, __syscall_lstat,
		const char *, file_name, struct kernel_stat *, buf);

int lstat(const char *file_name, struct stat *buf)
{
	int result;
	struct kernel_stat kbuf;

	result = __syscall_lstat(file_name, &kbuf);
	if (result == 0) {
		__xstat_conv(&kbuf, buf);
	}
	return result;
}

#if ! defined __NR_lstat64 && defined __UCLIBC_HAS_LFS__
weak_alias(lstat, lstat64);
#endif
