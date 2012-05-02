/* vi: set sw=4 ts=4: */
/*
 * geteuid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#ifdef	__NR_geteuid
#define __NR___syscall_geteuid __NR_geteuid
static inline _syscall0(int, __syscall_geteuid);
uid_t geteuid(void)
{
	return (__syscall_geteuid());
}
#else
uid_t geteuid(void)
{
	return (getuid());
}
#endif
