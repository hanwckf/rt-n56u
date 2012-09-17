/*
 * mknodat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>

#ifdef __NR_mknodat
int mknodat(int fd, const char *path, mode_t mode, dev_t dev)
{
	unsigned long long int k_dev;

	/* We must convert the value to dev_t type used by the kernel.  */
	k_dev = (dev) & ((1ULL << 32) - 1);

	return INLINE_SYSCALL(mknodat, 4, fd, path, mode, (unsigned int)k_dev);
}
libc_hidden_def(mknodat)
#else
/* should add emulation with mknod() and /proc/self/fd/ ... */
#endif
