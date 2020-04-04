/*
 * unlink() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#if defined __NR_unlinkat && !defined __NR_unlink
# include <fcntl.h>
int unlink(const char *pathname)
{
	return unlinkat(AT_FDCWD, pathname, 0);
}
#else
_syscall1(int, unlink, const char *, pathname)
#endif
libc_hidden_def(unlink)
