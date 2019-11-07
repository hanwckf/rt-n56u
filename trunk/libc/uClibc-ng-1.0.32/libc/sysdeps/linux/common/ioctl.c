/*
 * ioctl() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <cancel.h>

#define __NR___syscall_ioctl __NR_ioctl
static __always_inline
_syscall3(int, __syscall_ioctl, int, fd, unsigned long int, request, void *, arg)

int ioctl(int fd, unsigned long int request, ...)
{
	void *arg;
	va_list list;
#ifdef __NEW_THREADS
	int oldtype, result;
#endif

	va_start(list, request);
	arg = va_arg(list, void *);
	va_end(list);

	if (SINGLE_THREAD_P)
		return __syscall_ioctl(fd, request, arg);
#ifdef __NEW_THREADS
	oldtype = LIBC_CANCEL_ASYNC ();
	result = __syscall_ioctl(fd, request, arg);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
lt_strong_alias(ioctl)
lt_libc_hidden(ioctl)
