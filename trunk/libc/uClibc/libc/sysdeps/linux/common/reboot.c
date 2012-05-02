/* vi: set sw=4 ts=4: */
/*
 * _reboot() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#define __NR__reboot __NR_reboot
static inline _syscall3(int, _reboot, int, magic, int, magic2, int, flag);
int reboot(int flag)
{
	return (_reboot((int) 0xfee1dead, 672274793, flag));
}
