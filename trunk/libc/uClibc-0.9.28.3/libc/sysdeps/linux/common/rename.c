/* vi: set sw=4 ts=4: */
/*
 * rename() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <stdio.h>

#define __NR___syscall_rename __NR_rename
static inline _syscall2(int, __syscall_rename, const char *, oldpath,
		const char *, newpath);

int rename(const char * oldpath, const char * newpath)
{
	return __syscall_rename(oldpath, newpath);
}

