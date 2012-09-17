/* vi: set sw=4 ts=4: */
/*
 * _reboot() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/reboot.h>
#define __NR__reboot __NR_reboot
static __inline__ _syscall3(int, _reboot, int, magic, int, magic2, int, flag)
int reboot(int flag)
{
	return (_reboot((int) 0xfee1dead, 672274793, flag));
}
