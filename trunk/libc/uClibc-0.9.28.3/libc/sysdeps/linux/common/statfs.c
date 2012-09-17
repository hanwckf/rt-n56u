/* vi: set sw=4 ts=4: */
/*
 * statfs() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <string.h>
#include <sys/param.h>
#include <sys/vfs.h>

#define __NR___syscall_statfs __NR_statfs
static inline _syscall2(int, __syscall_statfs,
		const char *, path, struct statfs *, buf);

int statfs(const char *path, struct statfs * buf)
{
	return __syscall_statfs(path, buf);
}

