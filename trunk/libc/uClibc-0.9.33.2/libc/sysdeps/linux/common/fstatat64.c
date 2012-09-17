/*
 * fstatat64() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>
#include "xstatconv.h"

#ifdef __UCLIBC_HAS_LFS__

/* 64bit ports tend to favor newfstatat() */
#ifdef __NR_newfstatat
# define __NR_fstatat64 __NR_newfstatat
#endif

#ifdef __NR_fstatat64
int fstatat64(int fd, const char *file, struct stat64 *buf, int flag)
{
	int ret;
	struct kernel_stat64 kbuf;

	ret = INLINE_SYSCALL(fstatat64, 4, fd, file, &kbuf, flag);
	if (ret == 0)
		__xstat64_conv(&kbuf, buf);

	return ret;
}
#else
/* should add emulation with fstat64() and /proc/self/fd/ ... */
#endif

#endif
