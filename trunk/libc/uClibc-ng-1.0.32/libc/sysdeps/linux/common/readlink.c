/*
 * readlink() for uClibc
 *
 * Copyright (C) 2000-2007 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#if defined(__NR_readlinkat) && !defined(__NR_readlink)
# include <fcntl.h>
ssize_t readlink (const char *path, char *buf, size_t len)
{
	return readlinkat(AT_FDCWD, path, buf, len);
}
#else
_syscall3(ssize_t, readlink, const char *, path, char *, buf, size_t, bufsiz)
#endif
libc_hidden_def(readlink)
