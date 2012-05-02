/* vi: set sw=4 ts=4: */
/*
 * chdir() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <string.h>
#include <sys/param.h>

#define __NR___syscall_chdir __NR_chdir
static inline _syscall1(int, __syscall_chdir, const char *, path);
int chdir(const char *path)
{
	return __syscall_chdir(path);
}

