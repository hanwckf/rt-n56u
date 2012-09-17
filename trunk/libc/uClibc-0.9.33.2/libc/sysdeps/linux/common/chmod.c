/* vi: set sw=4 ts=4: */
/*
 * chmod() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>


#define __NR___syscall_chmod __NR_chmod
static __inline__ _syscall2(int, __syscall_chmod, const char *, path, __kernel_mode_t, mode)

int chmod(const char *path, mode_t mode)
{
	return __syscall_chmod(path, mode);
}
libc_hidden_def(chmod)
