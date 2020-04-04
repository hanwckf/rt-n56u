/*
 * stime() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifdef __USE_SVID
# include <time.h>
# ifdef __NR_stime
_syscall1(int, stime, const time_t *, t)
# elif defined __USE_BSD && defined __NR_settimeofday
#  define __need_NULL
#  include <stddef.h>
#  include <errno.h>
#  include <sys/time.h>
int stime(const time_t *when)
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
# endif
# if defined __NR_stime || (defined __USE_BSD && defined __NR_settimeofday)
libc_hidden_def(stime)
# endif
#endif
