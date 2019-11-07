/*
 * futimens() implementation for uClibc
 *
 * Copyright (C) 2009 Bernhard Reutner-Fischer <uclibc@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <time.h>
#ifdef __NR_utimensat
/* To avoid superfluous warnings about passing NULL to the non-null annotated
 * 2nd param "__path" below, we bypass inclusion of sys/stat.h and use
 * a non annotated, local decl.
 * Note that due to not including the header, we have to alias the call
 * manually.
 */
extern int utimensat (int __fd, const char *__path,
	const struct timespec __times[2],
	int __flags) __THROW;
libc_hidden_proto(utimensat)

int futimens (int __fd, const struct timespec __times[2]) __THROW;
int futimens (int fd, const struct timespec ts[2])
{
	return utimensat(fd, 0, ts, 0);
}
#endif
