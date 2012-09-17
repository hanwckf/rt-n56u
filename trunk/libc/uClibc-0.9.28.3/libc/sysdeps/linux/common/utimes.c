/* vi: set sw=4 ts=4: */
/*
 * utimes() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <utime.h>
#ifdef __NR_utimes
_syscall2(int, utimes, const char *, file, const struct timeval *, tvp);
#else
#include <stdlib.h>
#include <sys/time.h>
int utimes(const char *file, const struct timeval tvp[2])
{
	struct utimbuf buf, *times;

	if (tvp) {
		times = &buf;
		times->actime = tvp[0].tv_sec;
		times->modtime = tvp[1].tv_sec;
	} else {
		times = NULL;
	}
	return utime(file, times);
}
#endif
