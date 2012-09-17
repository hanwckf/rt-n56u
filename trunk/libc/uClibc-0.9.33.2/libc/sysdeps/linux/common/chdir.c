/* vi: set sw=4 ts=4: */
/*
 * chdir() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>


#define __NR___syscall_chdir __NR_chdir
static __inline__ _syscall1(int, __syscall_chdir, const char *, path)
int chdir(const char *path)
{
	return __syscall_chdir(path);
}
libc_hidden_def(chdir)
