/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#if defined __USE_BSD || defined __USE_POSIX98
#if defined __UCLIBC_HAS_REALTIME__

int usleep (__useconds_t usec)
{
	const struct timespec ts = {
		.tv_sec = (long int) (usec / 1000000),
		.tv_nsec = (long int) (usec % 1000000) * 1000ul
	};
	return nanosleep(&ts, NULL);
}
#else /* __UCLIBC_HAS_REALTIME__ */
int usleep (__useconds_t usec)
{
	struct timeval tv;

	tv.tv_sec = (long int) (usec / 1000000);
	tv.tv_usec = (long int) (usec % 1000000);
	return select(0, NULL, NULL, NULL, &tv);
}
#endif /* __UCLIBC_HAS_REALTIME__ */
#endif
