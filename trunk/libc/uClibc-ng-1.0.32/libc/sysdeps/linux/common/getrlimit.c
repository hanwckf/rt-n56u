/*
 * getrlimit() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/resource.h>
#include <bits/wordsize.h>

/* Only wrap getrlimit if the new ugetrlimit is not present and getrlimit sucks */

#if defined __NR_ugetrlimit

/* just call ugetrlimit() */
# define __NR___syscall_ugetrlimit __NR_ugetrlimit
static __always_inline
_syscall2(int, __syscall_ugetrlimit, enum __rlimit_resource, resource,
          struct rlimit *, rlim)
int getrlimit(__rlimit_resource_t resource, struct rlimit *rlimits)
{
	return __syscall_ugetrlimit(resource, rlimits);
}

#elif !defined(__UCLIBC_HANDLE_OLDER_RLIMIT__)

/* We don't need to wrap getrlimit() */
_syscall2(int, getrlimit, __rlimit_resource_t, resource,
	  struct rlimit *, rlim)

#else

/* we have to handle old style getrlimit() */
# define __NR___syscall_getrlimit __NR_getrlimit
static __always_inline
_syscall2(int, __syscall_getrlimit, int, resource, struct rlimit *, rlim)

int getrlimit(__rlimit_resource_t resource, struct rlimit *rlimits)
{
	int result;

	result = __syscall_getrlimit(resource, rlimits);

	if (result == -1)
		return result;

	/* We might have to correct the limits values.  Since the old values
	 * were signed the infinity value is too small.  */
	if (rlimits->rlim_cur == RLIM_INFINITY >> 1)
		rlimits->rlim_cur = RLIM_INFINITY;
	if (rlimits->rlim_max == RLIM_INFINITY >> 1)
		rlimits->rlim_max = RLIM_INFINITY;
	return result;
}
#endif
libc_hidden_def(getrlimit)

#if __WORDSIZE == 64
strong_alias_untyped(getrlimit, getrlimit64)
#endif
