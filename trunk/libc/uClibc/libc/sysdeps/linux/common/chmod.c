/* vi: set sw=4 ts=4: */
/*
 * chmod() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/stat.h>

#define __NR___syscall_chmod __NR_chmod
static inline _syscall2(int, __syscall_chmod, const char *, path, __kernel_mode_t, mode);

int chmod(const char *path, mode_t mode)
{
	return __syscall_chmod(path, mode);
}
