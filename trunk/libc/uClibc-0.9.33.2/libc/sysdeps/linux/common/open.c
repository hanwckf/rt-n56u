/* vi: set sw=4 ts=4: */
/*
 * open() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <sys/param.h>

#define __NR___syscall_open __NR_open
static __inline__ _syscall3(int, __syscall_open, const char *, file,
		int, flags, __kernel_mode_t, mode)

int open(const char *file, int oflag, ...)
{
	mode_t mode = 0;

	if (oflag & O_CREAT) {
		va_list arg;
		va_start(arg, oflag);
		mode = va_arg(arg, mode_t);
		va_end(arg);
	}

	return __syscall_open(file, oflag, mode);
}
#ifndef __LINUXTHREADS_OLD__
libc_hidden_def(open)
#else
libc_hidden_weak(open)
strong_alias(open,__libc_open)
#endif
