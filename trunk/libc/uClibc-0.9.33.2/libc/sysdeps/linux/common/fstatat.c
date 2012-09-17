/*
 * fstatat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>
#include "xstatconv.h"

/* 64bit ports tend to favor newfstatat() */
#ifdef __NR_newfstatat
# define __NR_fstatat64 __NR_newfstatat
#endif

#ifdef __NR_fstatat64
int fstatat(int fd, const char *file, struct stat *buf, int flag)
{
	int ret;
	struct kernel_stat64 kbuf;

	ret = INLINE_SYSCALL(fstatat64, 4, fd, file, &kbuf, flag);
	if (ret == 0)
		__xstat32_conv(&kbuf, buf);

	return ret;
}
#else
/* should add emulation with fstat() and /proc/self/fd/ ... */
#endif
