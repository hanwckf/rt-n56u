/*
 * utime() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <utime.h>

#if defined __NR_utimensat && !defined __NR_utime
# include <fcntl.h>
# include <stddef.h>

int utime(const char *file, const struct utimbuf *times)
{
	struct timespec tspecs[2], *ts;

	if (times) {
		ts = tspecs;
		ts[0].tv_sec = times->actime;
		ts[0].tv_nsec = 0;
		ts[1].tv_sec = times->modtime;
		ts[1].tv_nsec = 0;
	} else {
		ts = NULL;
	}

	return utimensat(AT_FDCWD, file, ts, 0);
}

#elif defined(__NR_utime)
_syscall2(int, utime, const char *, file, const struct utimbuf *, times)
#elif defined __NR_utimes /* alpha || ia64 */
# define __need_NULL
# include <stddef.h>
# include <sys/time.h>

int utime(const char *file, const struct utimbuf *times)
{
	struct timeval timevals[2];

	if (times != NULL) {
		timevals[0].tv_usec = 0L;
		timevals[1].tv_usec = 0L;
		timevals[0].tv_sec = (time_t) times->actime;
		timevals[1].tv_sec = (time_t) times->modtime;
	}
	return utimes(file, times ? timevals : NULL);
}
#endif

#if (defined __NR_utimensat && !defined __NR_utime) || \
	defined __NR_utime || defined __NR_utimes
libc_hidden_def(utime)
#endif
