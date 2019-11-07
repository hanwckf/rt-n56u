/*
 * open() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <fcntl.h>
#include <stdarg.h>
#include <cancel.h>

#if defined __NR_open
# define __NR___syscall_open __NR_open
static __always_inline _syscall3(int, __syscall_open, const char *, file,
				 int, flags, __kernel_mode_t, mode)
strong_alias_untyped(__syscall_open,__NC(open))

# define __NR___open2_nocancel __NR_open
_syscall2(int, __NC(open2), const char *, file, int, flags)
#else
int __open2_nocancel(const char *, int) __nonnull ((1)) attribute_hidden;
int __open_nocancel(const char *, int, mode_t) __nonnull ((1)) attribute_hidden;
#endif

int open(const char *file, int oflag, ...)
{
	mode_t mode = 0;
#ifdef __NEW_THREADS
	int oldtype, result;
#endif

	if (oflag & O_CREAT) {
		va_list arg;
		va_start(arg, oflag);
		mode = va_arg(arg, mode_t);
		va_end(arg);
	}

	if (SINGLE_THREAD_P)
#if defined(__NR_open)
		return __NC(open)(file, oflag, mode);
#elif defined(__NR_openat)
		return openat(AT_FDCWD, file, oflag, mode);
#endif

#ifdef __NEW_THREADS
	oldtype = LIBC_CANCEL_ASYNC ();
# if defined(__NR_open)
	result = __NC(open)(file, oflag, mode);
# else
	result = openat(AT_FDCWD, file, oflag, mode);
# endif
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
lt_strong_alias(open)
lt_libc_hidden(open)
#if !defined(__NR_open)
int __open2_nocancel(const char *file, int oflag)
{
	return open(file, oflag);
}
int __open_nocancel(const char *file, int oflag, mode_t mode)
{
	return open(file, oflag, mode);
}
#endif
