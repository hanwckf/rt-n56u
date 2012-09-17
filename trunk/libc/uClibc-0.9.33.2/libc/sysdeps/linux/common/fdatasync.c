/* vi: set sw=4 ts=4: */
/*
 * fdatasync() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#if !defined __NR_fdatasync && defined __NR_osf_fdatasync
# define __NR_fdatasync __NR_osf_fdatasync
#endif

#ifdef __NR_fdatasync

# ifdef __UCLIBC_HAS_THREADS_NATIVE__
# include <sysdep-cancel.h>
# else
# define SINGLE_THREAD_P 1
# endif

#define __NR___syscall_fdatasync __NR_fdatasync

static __always_inline
_syscall1(int, __syscall_fdatasync, int, fd)

int fdatasync(int fd)
{
	if (SINGLE_THREAD_P)
		return __syscall_fdatasync(fd);

# ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __syscall_fdatasync(fd);
	LIBC_CANCEL_RESET (oldtype);
	return result;
# endif
}

#endif
