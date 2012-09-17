/* vi: set sw=4 ts=4: */
/*
 * open() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <sys/param.h>

#define __NR___syscall_open __NR_open
static inline _syscall3(int, __syscall_open, const char *, file,
		int, flags, __kernel_mode_t, mode);

int __libc_open(const char *file, int flags, ...)
{
	/* gcc may warn about mode being uninitialized.
	 * Just ignore that, since gcc is wrong. */
	mode_t mode = 0;

	if (flags & O_CREAT) {
		va_list ap;

		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}
	return __syscall_open(file, flags, mode);
}
weak_alias(__libc_open, open);

int creat(const char *file, mode_t mode)
{
	return __libc_open(file, O_WRONLY | O_CREAT | O_TRUNC, mode);
}
