/* vi: set sw=4 ts=4: */
/*
 * ioperm() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#if defined __ARCH_HAS_MMU__ && defined __NR_ioperm
_syscall3(int, ioperm, unsigned long, from, unsigned long, num, int, turn_on);
#else
int ioperm(unsigned long from, unsigned long num, int turn_on)
{
	__set_errno(ENOSYS);
	return -1;
}
#endif
