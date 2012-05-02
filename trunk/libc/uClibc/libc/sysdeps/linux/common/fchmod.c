/* vi: set sw=4 ts=4: */
/*
 * fchmod() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/stat.h>

#define __NR___syscall_fchmod __NR_fchmod
static inline _syscall2(int, __syscall_fchmod,
		int, fildes, __kernel_mode_t, mode);

int fchmod(int fildes, mode_t mode)
{
	return (__syscall_fchmod(fildes, mode));
}
