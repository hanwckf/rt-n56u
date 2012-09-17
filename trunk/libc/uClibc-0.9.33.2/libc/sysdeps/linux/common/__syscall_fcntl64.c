/* vi: set sw=4 ts=4: */
/*
 * __syscall_fcntl64() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <stdarg.h>
#include <fcntl.h>

#if defined __UCLIBC_HAS_LFS__ && defined __NR_fcntl64

#define __NR___syscall_fcntl64 __NR_fcntl64
static __inline__ _syscall3(int, __syscall_fcntl64, int, fd, int, cmd, long, arg)
int fcntl64(int fd, int cmd, ...)
{
	long arg;
	va_list list;

	va_start(list, cmd);
	arg = va_arg(list, long);
	va_end(list);

	return (__syscall_fcntl64(fd, cmd, arg));
}
libc_hidden_def(fcntl64)
#endif
