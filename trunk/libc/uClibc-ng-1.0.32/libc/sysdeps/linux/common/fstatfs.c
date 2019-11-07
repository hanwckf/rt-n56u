/*
 * fstatfs() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/vfs.h>
#include <string.h>

#ifndef __USE_FILE_OFFSET64__
extern int fstatfs (int __fildes, struct statfs *__buf)
     __THROW __nonnull ((2));
#else
# ifdef __REDIRECT_NTH
extern int __REDIRECT_NTH (fstatfs, (int __fildes, struct statfs *__buf),
	fstatfs64) __nonnull ((2));
# else
#  define fstatfs fstatfs64
# endif
#endif

extern __typeof(fstatfs) __libc_fstatfs attribute_hidden;
#ifdef __NR_fstatfs
# define __NR___libc_fstatfs __NR_fstatfs
_syscall2(int, __libc_fstatfs, int, fd, struct statfs *, buf)
#else
int __libc_fstatfs (int __fildes, struct statfs *__buf)
{
	int err = INLINE_SYSCALL(fstatfs64, 3, __fildes, sizeof(*__buf), __buf);
	return err;
};
/* Redefined fstatfs because we need it for backwards compatibility */
#endif /* __NR_fstatfs */

#if defined __UCLIBC_LINUX_SPECIFIC__
weak_alias(__libc_fstatfs,fstatfs)
#endif
