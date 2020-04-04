/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdarg.h>
#include <cancel.h>

/* Open FILE with access OFLAG.  If OFLAG includes O_CREAT,
   a third argument is the file protection.  */
int open64(const char *file, int oflag, ...)
{
	mode_t mode = 0;

	if (oflag & O_CREAT) {
		va_list arg;
		va_start (arg, oflag);
		mode = va_arg (arg, mode_t);
		va_end (arg);
	}
#if defined __NR_openat && !defined __NR_open
	return openat(AT_FDCWD, file, oflag | O_LARGEFILE, mode);
#else
	return open(file, oflag | O_LARGEFILE, mode);
#endif
}
lt_strong_alias(open64)
lt_libc_hidden(open64)
/* open handled cancellation, noop on uClibc */
LIBC_CANCEL_HANDLED();
