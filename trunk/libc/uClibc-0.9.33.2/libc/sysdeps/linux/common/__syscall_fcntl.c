/* vi: set sw=4 ts=4: */
/*
 * __syscall_fcntl() for uClibc
 *
 * Copyright (C) 2006 Steven J. Hill <sjhill@realitydiluted.com>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <stdarg.h>
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <sysdep-cancel.h>	/* Must come before <fcntl.h>.  */
#endif
#include <fcntl.h>
#include <bits/wordsize.h>

extern __typeof(fcntl) __libc_fcntl;
libc_hidden_proto(__libc_fcntl)

int __fcntl_nocancel (int fd, int cmd, ...)
{
	va_list ap;
	void *arg;

	va_start (ap, cmd);
	arg = va_arg (ap, void *);
	va_end (ap);

# if __WORDSIZE == 32
	if (cmd == F_GETLK64 || cmd == F_SETLK64 || cmd == F_SETLKW64) {
#  if defined __UCLIBC_HAS_LFS__ && defined __NR_fcntl64
		return INLINE_SYSCALL (fcntl64, 3, fd, cmd, arg);
#  else
		__set_errno(ENOSYS);
		return -1;
#  endif
	}
# endif
	return INLINE_SYSCALL (fcntl, 3, fd, cmd, arg);
}
libc_hidden_def(__fcntl_nocancel)

int __libc_fcntl (int fd, int cmd, ...)
{
	va_list ap;
	void *arg;

	va_start (ap, cmd);
	arg = va_arg (ap, void *);
	va_end (ap);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	if (SINGLE_THREAD_P || (cmd != F_SETLKW && cmd != F_SETLKW64))
# if defined __UCLIBC_HAS_LFS__ && defined __NR_fcntl64
		return INLINE_SYSCALL (fcntl64, 3, fd, cmd, arg);
# else
		return INLINE_SYSCALL (fcntl, 3, fd, cmd, arg);
# endif

	int oldtype = LIBC_CANCEL_ASYNC ();

# if defined __UCLIBC_HAS_LFS__ && defined __NR_fcntl64
	int result = INLINE_SYSCALL (fcntl64, 3, fd, cmd, arg);
# else
	int result = INLINE_SYSCALL (fcntl, 3, fd, cmd, arg);
# endif

	LIBC_CANCEL_RESET (oldtype);

	return result;
#else
# if __WORDSIZE == 32
	if (cmd == F_GETLK64 || cmd == F_SETLK64 || cmd == F_SETLKW64) {
#  if defined __UCLIBC_HAS_LFS__ && defined __NR_fcntl64
		return INLINE_SYSCALL (fcntl64, 3, fd, cmd, arg);
#  else
		__set_errno(ENOSYS);
		return -1;
#  endif
	}
# endif
	return INLINE_SYSCALL (fcntl, 3, fd, cmd, arg);
#endif
}
libc_hidden_def(__libc_fcntl)

libc_hidden_proto(fcntl)
weak_alias(__libc_fcntl,fcntl)
libc_hidden_weak(fcntl)
