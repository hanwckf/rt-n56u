/* vi: set sw=4 ts=4: */
/*
 * setrlimit() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define setrlimit64 __hide_setrlimit64
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/resource.h>
#undef setrlimit64


/* Only wrap setrlimit if the new usetrlimit is not present and setrlimit sucks */

#if defined(__NR_usetrlimit)

/* just call usetrlimit() */
# define __NR___syscall_usetrlimit __NR_usetrlimit
static __always_inline
_syscall2(int, __syscall_usetrlimit, enum __rlimit_resource, resource,
          const struct rlimit *, rlim)
int setrlimit(__rlimit_resource_t resource, struct rlimit *rlimits)
{
	return (__syscall_usetrlimit(resource, rlimits));
}

#elif !defined(__UCLIBC_HANDLE_OLDER_RLIMIT__)

/* We don't need to wrap setrlimit() */
_syscall2(int, setrlimit, __rlimit_resource_t, resource,
		const struct rlimit *, rlim)

#else

/* we have to handle old style setrlimit() */
# define __NR___syscall_setrlimit __NR_setrlimit
static __always_inline
_syscall2(int, __syscall_setrlimit, int, resource, const struct rlimit *, rlim)

int setrlimit(__rlimit_resource_t resource, const struct rlimit *rlimits)
{
	struct rlimit rlimits_small;

	if (rlimits == NULL) {
		__set_errno(EINVAL);
		return -1;
	}

	/* We might have to correct the limits values.  Since the old values
	 * were signed the new values might be too large.  */
# define RMIN(x, y) ((x) < (y) ? (x) : (y))
	rlimits_small.rlim_cur = RMIN((unsigned long int) rlimits->rlim_cur,
								  RLIM_INFINITY >> 1);
	rlimits_small.rlim_max = RMIN((unsigned long int) rlimits->rlim_max,
								  RLIM_INFINITY >> 1);
#undef RMIN
	return (__syscall_setrlimit(resource, &rlimits_small));
}
#endif

libc_hidden_def(setrlimit)

#if defined __UCLIBC_HAS_LFS__ && __WORDSIZE == 64
strong_alias(setrlimit, setrlimit64)
#endif
