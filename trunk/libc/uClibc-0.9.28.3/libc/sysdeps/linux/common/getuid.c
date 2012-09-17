/* vi: set sw=4 ts=4: */
/*
 * getuid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#if defined (__alpha__)
#define __NR_getuid     __NR_getxuid
#endif
#define __NR___syscall_getuid __NR_getuid

static inline _syscall0(int, __syscall_getuid);

uid_t getuid(void)
{
	return (__syscall_getuid());
}
