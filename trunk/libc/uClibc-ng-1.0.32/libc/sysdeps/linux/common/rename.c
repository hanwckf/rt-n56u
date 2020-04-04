/*
 * rename() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>

#if defined __NR_renameat && !defined __NR_rename
# include <fcntl.h>
int rename(const char *oldpath, const char *newpath)
{
	return renameat(AT_FDCWD, oldpath, AT_FDCWD, newpath);
}
#elif defined __NR_renameat2
# include <fcntl.h>
int rename(const char *oldpath, const char *newpath)
{
	_syscall5(int, renameat2, int, olddfd, const char *, oldpath,
		  int, newdfd, const char *, newpath, int, flags)
	return renameat2(AT_FDCWD, oldpath, AT_FDCWD, newpath, 0);
}
#else
_syscall2(int, rename, const char *, oldpath, const char *, newpath)
#endif
