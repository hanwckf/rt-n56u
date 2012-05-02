/* vi: set sw=4 ts=4: */
/*
 * setrlimit() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"

#ifndef __NR_ugetrlimit
/* Only wrap setrlimit if the new ugetrlimit is not present */

#define __NR___setrlimit __NR_setrlimit
#include <unistd.h>
#include <sys/resource.h>
#define RMIN(x, y) ((x) < (y) ? (x) : (y))
_syscall2(int, __setrlimit, int, resource, const struct rlimit *, rlim);
int setrlimit(__rlimit_resource_t resource, const struct rlimit *rlimits)
{
	struct rlimit rlimits_small;

	/* We might have to correct the limits values.  Since the old values
	 * were signed the new values might be too large.  */
	rlimits_small.rlim_cur = RMIN((unsigned long int) rlimits->rlim_cur,
								  RLIM_INFINITY >> 1);
	rlimits_small.rlim_max = RMIN((unsigned long int) rlimits->rlim_max,
								  RLIM_INFINITY >> 1);
	return (__setrlimit(resource, &rlimits_small));
}

#undef RMIN

#else							/* We don't need to wrap setrlimit */

#include <unistd.h>
struct rlimit;
_syscall2(int, setrlimit, unsigned int, resource,
		const struct rlimit *, rlim);
#endif
