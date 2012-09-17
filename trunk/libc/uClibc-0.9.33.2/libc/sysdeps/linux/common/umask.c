/* vi: set sw=4 ts=4: */
/*
 * umask() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>

#define __NR___syscall_umask __NR_umask
static __inline__ _syscall1(__kernel_mode_t, __syscall_umask, __kernel_mode_t, mode)

mode_t umask(mode_t mode)
{
	return __syscall_umask(mode);
}
