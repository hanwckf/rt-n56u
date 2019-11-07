/*
 * mkdir() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>

#if defined __NR_mkdirat && !defined __NR_mkdir
# include <fcntl.h>
int mkdir(const char *pathname, mode_t mode)
{
	return mkdirat(AT_FDCWD, pathname, mode);
}

#else
# define __NR___syscall_mkdir __NR_mkdir
static __inline__ _syscall2(int, __syscall_mkdir, const char *, pathname,
		__kernel_mode_t, mode)

int mkdir(const char *pathname, mode_t mode)
{
	return (__syscall_mkdir(pathname, mode));
}
#endif
libc_hidden_def(mkdir)
