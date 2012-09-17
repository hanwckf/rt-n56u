/* vi: set sw=4 ts=4: */
/*
 * nanosleep() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <time.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <sysdep-cancel.h>
#include <pthreadP.h>
#else
#define SINGLE_THREAD_P 1
#endif

#define __NR___syscall_nanosleep __NR_nanosleep
static __always_inline _syscall2(int, __syscall_nanosleep, const struct timespec *, req,
						struct timespec *, rem);

extern __typeof(nanosleep) __libc_nanosleep;

int __libc_nanosleep(const struct timespec *req, struct timespec *rem)
{
	if (SINGLE_THREAD_P)
		return __syscall_nanosleep(req, rem);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __syscall_nanosleep(req, rem);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}

weak_alias(__libc_nanosleep,nanosleep)
libc_hidden_weak(nanosleep)
