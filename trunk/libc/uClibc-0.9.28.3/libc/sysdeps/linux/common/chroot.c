/* vi: set sw=4 ts=4: */
/*
 * chroot() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#include <string.h>
#include <sys/param.h>

#define __NR___syscall_chroot __NR_chroot
static inline _syscall1(int, __syscall_chroot, const char *, path);

int chroot(const char *path)
{
	return __syscall_chroot(path);
}
