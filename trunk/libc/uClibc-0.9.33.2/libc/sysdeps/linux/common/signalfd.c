/* vi: set sw=4 ts=4: */
/*
 * signalfd() for uClibc
 *
 * Copyright (C) 2008 Bernhard Reutner-Fischer <uclibc@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <signal.h>
#include <sys/signalfd.h>

#if defined __NR_signalfd4
#define __NR___syscall_signalfd4 __NR_signalfd4
static __inline__ _syscall4(int, __syscall_signalfd4, int, fd,
		const sigset_t *, mask, size_t, sizemask, int, flags)
#elif defined __NR_signalfd
#define __NR___syscall_signalfd __NR_signalfd
static __inline__ _syscall3(int, __syscall_signalfd, int, fd,
		const sigset_t *, mask, size_t, sizemask)
#endif

#if defined __NR_signalfd4 || defined __NR_signalfd
int signalfd (int fd, const sigset_t *mask, int flags)
{
#if defined __NR___syscall_signalfd4
	return __syscall_signalfd4(fd, mask, _NSIG / 8, flags);
#elif defined __NR___syscall_signalfd
	if (flags != 0) {
		__set_errno(EINVAL);
		return -1;
	}
	return __syscall_signalfd(fd, mask, _NSIG / 8);
#endif
}
#endif
