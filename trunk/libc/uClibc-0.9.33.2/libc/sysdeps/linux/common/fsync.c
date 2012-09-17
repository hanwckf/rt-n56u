/* vi: set sw=4 ts=4: */
/*
 * fsync() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include "sysdep-cancel.h"
#else
#define SINGLE_THREAD_P 1
#endif

#define __NR___syscall_fsync __NR_fsync
static __always_inline _syscall1(int, __syscall_fsync, int, fd)

extern __typeof(fsync) __libc_fsync;

int __libc_fsync(int fd)
{
	if (SINGLE_THREAD_P)
		return __syscall_fsync(fd);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __syscall_fsync(fd);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}

weak_alias(__libc_fsync, fsync)
