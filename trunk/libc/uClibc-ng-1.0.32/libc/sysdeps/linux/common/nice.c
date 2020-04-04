/*
 * nice() for uClibc
 *
 * Copyright (C) 2005 by Manuel Novoa III <mjn3@codepoet.org>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <sys/resource.h>


#ifdef __NR_nice

# define __NR___syscall_nice __NR_nice
static __inline__ _syscall1(int, __syscall_nice, int, incr)

#else

# include <limits.h>


static __inline__ int int_add_no_wrap(int a, int b)
{
	if (b < 0) {
		if (a < INT_MIN - b)
			return INT_MIN;
	} else {
		if (a > INT_MAX - b)
			return INT_MAX;
	}

	return a + b;
}

static __inline__ int __syscall_nice(int incr)
{
	int old_priority;
# if 1
	/* This should never fail. */
	old_priority = getpriority(PRIO_PROCESS, 0);
# else
	/* But if you want to be paranoid... */
	int old_errno;

	old_errno = errno;
	__set_errno(0);
	old_priority = getpriority(PRIO_PROCESS, 0);
	if ((old_priority == -1) && errno) {
		return -1;
	}
	__set_errno(old_errno);
# endif

	if (setpriority(PRIO_PROCESS, 0, int_add_no_wrap(old_priority, incr))) {
		__set_errno(EPERM);	/* SUSv3 mandates EPERM for nice failure. */
		return -1;
	}

	return 0;
}

#endif

int nice(int incr)
{
	if (__syscall_nice(incr)) {
		return -1;
	}

	return getpriority(PRIO_PROCESS, 0);
}
