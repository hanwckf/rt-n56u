/* vi: set sw=4 ts=4: */
/*
 * iopl() for uClibc
 *
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
/* Tuns out the m68k unistd.h kernel header is broken */
#if defined __ARCH_HAS_MMU__ && defined __NR_iopl && ( !defined(__mc68000__))
_syscall1(int, iopl, int, level);
#else
int iopl(int level)
{
	__set_errno(ENOSYS);
	return -1;
}
#endif
