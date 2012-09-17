/* vi: set sw=4 ts=4: */
/*
 * stime() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <time.h>
#include <sys/time.h>

#ifdef __USE_SVID
#ifdef __NR_stime
_syscall1(int, stime, const time_t *, t)
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
#endif
