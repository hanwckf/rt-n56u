/* vi: set sw=4 ts=4: */
/*
 * fchmod() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>

#define __NR___syscall_fchmod __NR_fchmod
static __inline__ _syscall2(int, __syscall_fchmod,
		int, fildes, __kernel_mode_t, mode)

int fchmod(int fildes, mode_t mode)
{
	return (__syscall_fchmod(fildes, mode));
}
