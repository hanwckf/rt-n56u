/*
 * chmod() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined __NR_fchmodat && !defined __NR_chmod
# include <fcntl.h>
int chmod(const char *path, mode_t mode)
{
	return fchmodat(AT_FDCWD, path, mode, 0);
}

#else
# define __NR___syscall_chmod __NR_chmod
static __inline__ _syscall2(int, __syscall_chmod, const char *, path, __kernel_mode_t, mode)

int chmod(const char *path, mode_t mode)
{
	return __syscall_chmod(path, mode);
}
#endif
libc_hidden_def(chmod)
