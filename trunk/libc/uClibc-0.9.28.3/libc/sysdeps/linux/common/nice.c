/* vi: set sw=4 ts=4: */
/*
 * nice() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2005 by Manuel Novoa III <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#include <sys/resource.h>

#ifdef __NR_nice

#define __NR___syscall_nice __NR_nice
static inline _syscall1(int, __syscall_nice, int, incr);

#else

#include <limits.h>

static inline int int_add_no_wrap(int a, int b)
{
	int s = a + b;

	if (b < 0) {
		if (s > a) s = INT_MIN;
	} else {
		if (s < a) s = INT_MAX;
	}

	return s;
}

static inline int __syscall_nice(int incr)
{
	int old_priority;
#if 1
	/* This should never fail. */
	old_priority = getpriority(PRIO_PROCESS, 0);
#else
	/* But if you want to be paranoid... */
	int old_errno;

	old_errno = errno;
	__set_errno(0);
	old_priority = getpriority(PRIO_PROCESS, 0);
	if ((old_priority == -1) && errno) {
		return -1;
	}
	__set_errno(old_errno);
#endif

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
