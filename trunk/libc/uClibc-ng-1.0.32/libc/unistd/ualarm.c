/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


useconds_t ualarm(useconds_t value, useconds_t interval)
{
    struct itimerval otimer;
    const struct itimerval itimer = {
	{ 0, interval },
	{ 0, value}
    };

    if (setitimer(ITIMER_REAL, &itimer, &otimer) < 0) {
	return -1;
    }
    return((otimer.it_value.tv_sec * 1000000) + otimer.it_value.tv_usec);
}
