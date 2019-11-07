/*
 * symlink() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#if defined __USE_BSD || defined __USE_UNIX98 || defined __USE_XOPEN2K
# include <unistd.h>

# if defined __NR_symlinkat && !defined __NR_symlink
#  include <fcntl.h>
int symlink(const char *oldpath, const char *newpath)
{
	return symlinkat(oldpath, AT_FDCWD, newpath);
}

# elif defined(__NR_symlink)

_syscall2(int, symlink, const char *, oldpath, const char *, newpath)

# endif

#endif
