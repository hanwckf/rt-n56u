/* vi: set sw=4 ts=4: */
/*
 * stime() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <time.h>
#include <sys/time.h>
#ifdef __NR_stime
_syscall1(int, stime, const time_t *, t);
#else
int stime(const time_t * when)
{
	struct timeval tv;

	if (when == NULL) {
		__set_errno(EINVAL);
		return -1;
	}
	tv.tv_sec = *when;
	tv.tv_usec = 0;
	return settimeofday(&tv, (struct timezone *) 0);
}
#endif
