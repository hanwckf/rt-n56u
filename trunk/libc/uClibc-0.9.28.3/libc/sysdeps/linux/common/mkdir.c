/* vi: set sw=4 ts=4: */
/*
 * mkdir() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/stat.h>

#define __NR___syscall_mkdir __NR_mkdir
static inline _syscall2(int, __syscall_mkdir, const char *, pathname,
		__kernel_mode_t, mode);

int mkdir(const char *pathname, mode_t mode)
{
	return (__syscall_mkdir(pathname, mode));
}
