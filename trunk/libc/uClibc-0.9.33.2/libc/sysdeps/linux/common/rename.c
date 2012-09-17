/* vi: set sw=4 ts=4: */
/*
 * rename() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <stdio.h>

#define __NR___syscall_rename __NR_rename
static __inline__ _syscall2(int, __syscall_rename, const char *, oldpath,
		const char *, newpath)

int rename(const char * oldpath, const char * newpath)
{
	return __syscall_rename(oldpath, newpath);
}

