/* vi: set sw=4 ts=4: */
/*
 * chroot() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>

#if defined __USE_BSD || (defined __USE_XOPEN && !defined __USE_XOPEN2K)
#define __NR___syscall_chroot __NR_chroot
static __inline__ _syscall1(int, __syscall_chroot, const char *, path)

int chroot(const char *path)
{
	return __syscall_chroot(path);
}
#endif
