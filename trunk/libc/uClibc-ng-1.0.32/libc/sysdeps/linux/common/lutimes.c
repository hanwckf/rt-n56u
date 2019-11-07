/*
 * lutimes() implementation for uClibc
 *
 * Copyright (C) 2010 Vladimir Zapolskiy <vzapolskiy@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <time.h>

#ifdef __NR_lutimes
_syscall2(int, lutimes, const char *, file, const struct timeval *, tvp)
#elif defined __NR_utimensat
#include <sys/time.h>
#include <fcntl.h>

int lutimes(const char *file, const struct timeval tvp[2])
{
	struct timespec ts[2];

	if (tvp != NULL)
	{
		if (tvp[0].tv_usec < 0 || tvp[0].tv_usec >= 1000000
		    || tvp[1].tv_usec < 0 || tvp[1].tv_usec >= 1000000)
		{
			__set_errno(EINVAL);
			return -1;
		}

		TIMEVAL_TO_TIMESPEC(&tvp[0], &ts[0]);
		TIMEVAL_TO_TIMESPEC(&tvp[1], &ts[1]);
	}

	return utimensat(AT_FDCWD, file, tvp ? ts : NULL, AT_SYMLINK_NOFOLLOW);
}
#endif
