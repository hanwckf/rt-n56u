/* vi: set sw=4 ts=4: */
/*
 * msync() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>

#ifdef __NR_msync

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <sysdep-cancel.h>
#else
#define SINGLE_THREAD_P 1
#endif

#define __NR___syscall_msync __NR_msync
static __always_inline _syscall3(int, __syscall_msync, void *, addr, size_t, length,
						int, flags)

extern __typeof(msync) __libc_msync;
int __libc_msync(void * addr, size_t length, int flags)
{
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype, result;
#endif

	if (SINGLE_THREAD_P)
		return __syscall_msync(addr, length, flags);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	oldtype = LIBC_CANCEL_ASYNC ();
	result = __syscall_msync(addr, length, flags);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
weak_alias(__libc_msync,msync)

#endif
