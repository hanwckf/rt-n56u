/* vi: set sw=4 ts=4: */
/*
 * umask() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/stat.h>

#define __NR___syscall_umask __NR_umask
static inline _syscall1(__kernel_mode_t, __syscall_umask, __kernel_mode_t, mode);

mode_t umask(mode_t mode)
{
	return (__syscall_umask(mode));
}
