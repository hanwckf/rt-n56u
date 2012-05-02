/* vi: set sw=4 ts=4: */
/*
 * getgid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#define __NR___syscall_getgid __NR_getgid
#if defined (__alpha__)
#define __NR_getgid     __NR_getxgid
#endif

static inline _syscall0(int, __syscall_getgid);
gid_t getgid(void)
{
	return (__syscall_getgid());
}
