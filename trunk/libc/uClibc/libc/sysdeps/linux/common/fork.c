/* vi: set sw=4 ts=4: */
/*
 * fork() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#ifdef __ARCH_HAS_MMU__
#define __NR___libc_fork __NR_fork
_syscall0(pid_t, __libc_fork);
#else
pid_t __libc_fork(void)
{
	__set_errno(ENOSYS);
	return -1;
}
#endif
weak_alias(__libc_fork, fork);
