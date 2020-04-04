/*
 * mknod() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>

#if defined __NR_mknodat && !defined __NR_mknod
# include <fcntl.h>
int mknod(const char *path, mode_t mode, dev_t dev)
{
	return mknodat(AT_FDCWD, path, mode, dev);
}
#else
int mknod(const char *path, mode_t mode, dev_t dev)
{
	unsigned long long int k_dev;

	/* We must convert the value to dev_t type used by the kernel.  */
	k_dev = (dev) & ((1ULL << 32) - 1);

	if (k_dev != dev) {
		__set_errno(EINVAL);
		return -1;
	}
	return INLINE_SYSCALL(mknod, 3, path, mode, (unsigned int)k_dev);
}
#endif
libc_hidden_def(mknod)
