/*
 * dup2() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#if defined __NR_dup3 && !defined __NR_dup2
# include <fcntl.h>
extern int __libc_fcntl (int fd, int cmd, ...);
libc_hidden_proto(__libc_fcntl);

int dup2(int old, int newfd)
{
	/*
	 * Check if old fd is valid before we try
	 * to ducplicate it. Return it if valid
	 * or EBADF otherwise
	 */
	if (old == newfd)
		return fcntl(old, F_GETFL, 0) < 0 ? -1 : newfd;

	return dup3(old, newfd, 0);
}
#else
_syscall2(int, dup2, int, oldfd, int, newfd)
#endif
libc_hidden_def(dup2)
