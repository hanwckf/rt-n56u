/* vi: set sw=4 ts=4: */
/*
 * _sysctl() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#if defined __NR__sysctl && (defined __USE_GNU || defined __USE_BSD)

/* psm: including sys/sysctl.h would depend on kernel headers */
struct __sysctl_args {
	int *name;
	int nlen;
	void *oldval;
	size_t *oldlenp;
	void *newval;
	size_t newlen;
	unsigned long __unused[4];
};
extern int sysctl (int *__name, int __nlen, void *__oldval,
				   size_t *__oldlenp, void *__newval, size_t __newlen) __THROW;
int sysctl(int *name, int nlen, void *oldval, size_t * oldlenp,
		   void *newval, size_t newlen)
{
	/* avoid initializing on the stack as gcc will call memset() */
	struct __sysctl_args args;
	args.name = name;
	args.nlen = nlen;
	args.oldval = oldval;
	args.oldlenp = oldlenp;
	args.newval = newval;
	args.newlen = newlen;
	return INLINE_SYSCALL(_sysctl, 1, &args);
}
#endif
