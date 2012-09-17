/* vi: set sw=4 ts=4: */
/*
 * lseek() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_lseek
_syscall3(__off_t, lseek, int, fildes, __off_t, offset, int, whence)
#else

__off_t lseek(int fildes, __off_t offset, int whence)
{
	return lseek64(fildes, offset, whence);
}
#endif
#ifndef __LINUXTHREADS_OLD__
libc_hidden_def(lseek)
#else
libc_hidden_weak(lseek)
strong_alias(lseek,__libc_lseek)
#endif
