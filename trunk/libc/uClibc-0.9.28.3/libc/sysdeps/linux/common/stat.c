/* vi: set sw=4 ts=4: */
/*
 * stat() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#define _SYS_STAT_H
#include <bits/stat.h>
#include "xstatconv.h"

#define __NR___syscall_stat __NR_stat
static inline _syscall2(int, __syscall_stat,
		const char *, file_name, struct kernel_stat *, buf);

int stat(const char *file_name, struct stat *buf)
{
	int result;
	struct kernel_stat kbuf;

	result = __syscall_stat(file_name, &kbuf);
	if (result == 0) {
		__xstat_conv(&kbuf, buf);
	}
	return result;
}

#if ! defined __NR_stat64 && defined __UCLIBC_HAS_LFS__
weak_alias(stat, stat64);
#endif
