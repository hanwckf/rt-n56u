/* vi: set sw=4 ts=4: */
/*
 * capget() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#	ifdef __NR_capget
_syscall2(int, capget, void *, header, void *, data);
#	else
int capget(void *header, void *data)
{
	__set_errno(ENOSYS);
	return -1;
}
#	endif
