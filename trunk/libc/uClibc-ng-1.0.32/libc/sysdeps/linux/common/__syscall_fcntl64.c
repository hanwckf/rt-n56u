/*
 * __syscall_fcntl64() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <bits/wordsize.h>

#if defined __NR_fcntl64 && __WORDSIZE == 32
# include <stdarg.h>
# include <cancel.h>
# include <fcntl.h>

# define __NR___fcntl64_nocancel __NR_fcntl64
_syscall3(int, __NC(fcntl64), int, fd, int, cmd, long, arg)

int fcntl64(int fd, int cmd, ...)
{
	long arg;
	va_list list;
# ifdef __NEW_THREADS
	int oldtype, result;
# endif

	va_start(list, cmd);
	arg = va_arg(list, long);
	va_end(list);

	if (SINGLE_THREAD_P || (cmd != F_SETLKW && cmd != F_SETLKW64))
		return __NC(fcntl64)(fd, cmd, arg);
# ifdef __NEW_THREADS
	oldtype = LIBC_CANCEL_ASYNC();
	result = __NC(fcntl64)(fd, cmd, arg);
	LIBC_CANCEL_RESET(oldtype);
	return result;
# endif
}
lt_strong_alias(fcntl64)
lt_libc_hidden(fcntl64)
#endif
