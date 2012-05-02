/* vi: set sw=4 ts=4: */
/*
 * alarm() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#ifdef __NR_alarm
_syscall1(unsigned int, alarm, unsigned int, seconds);
#else
#include <sys/time.h>
unsigned int alarm(unsigned int seconds)
{
	struct itimerval old, new;
	unsigned int retval;

	new.it_value.tv_usec = 0;
	new.it_interval.tv_sec = 0;
	new.it_interval.tv_usec = 0;
	new.it_value.tv_sec = (long int) seconds;
	if (setitimer(ITIMER_REAL, &new, &old) < 0) {
		return 0;
	}
	retval = old.it_value.tv_sec;
	if (old.it_value.tv_usec) {
		++retval;
	}
	return retval;
}
#endif
