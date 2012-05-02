/* vi: set sw=4 ts=4: */
/*
 * flock() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/file.h>

#define __NR___syscall_flock __NR_flock
static inline _syscall2(int, __syscall_flock, int, fd, int, operation);

int flock(int fd, int operation)
{
	return (__syscall_flock(fd, operation));
}
