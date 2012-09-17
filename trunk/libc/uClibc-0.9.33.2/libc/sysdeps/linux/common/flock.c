/* vi: set sw=4 ts=4: */
/*
 * flock() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/file.h>

#define __NR___syscall_flock __NR_flock
static __inline__ _syscall2(int, __syscall_flock, int, fd, int, operation)

int flock(int fd, int operation)
{
	return (__syscall_flock(fd, operation));
}
