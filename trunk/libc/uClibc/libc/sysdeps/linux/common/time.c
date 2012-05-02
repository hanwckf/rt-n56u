/* vi: set sw=4 ts=4: */
/*
 * time() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <time.h>
#include <sys/time.h>
#ifdef __NR_time
_syscall1(time_t, time, time_t *, t);
#else
time_t time(time_t * t)
{
	time_t result;
	struct timeval tv;

	if (gettimeofday(&tv, (struct timezone *) NULL)) {
		result = (time_t) - 1;
	} else {
		result = (time_t) tv.tv_sec;
	}
	if (t != NULL) {
		*t = result;
	}
	return result;
}
#endif
