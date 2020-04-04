/*
 * openat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <fcntl.h>

#ifdef __NR_openat
static int __openat64(int fd, const char *file, int oflag, mode_t mode)
{
	return openat(fd, file, oflag | O_LARGEFILE, mode);
}
strong_alias_untyped(__openat64,openat64)
#else
/* should add emulation with open() and /proc/self/fd/ ... */
#endif
