/* vi: set sw=4 ts=4: */
/*
 * ustat() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/ustat.h>
#include <sys/sysmacros.h>

#define __NR___syscall_ustat __NR_ustat
/* Kernel's fs/super.c defines this:
 * long sys_ustat(unsigned dev, struct ustat __user * ubuf),
 * thus we use unsigned, not __kernel_dev_t.
 */
static __inline__ _syscall2(int, __syscall_ustat,
		unsigned, kdev_t,
		struct ustat *, ubuf)

int ustat(dev_t dev, struct ustat *ubuf)
{
	return __syscall_ustat(dev, ubuf);
}
