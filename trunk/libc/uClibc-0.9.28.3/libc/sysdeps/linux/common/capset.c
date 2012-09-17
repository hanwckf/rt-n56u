/* vi: set sw=4 ts=4: */
/*
 * capset() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#	ifdef __NR_capset
_syscall2(int, capset, void *, header, const void *, data);
#	else
int capset(void *header, const void *data)
{
	__set_errno(ENOSYS);
	return -1;
}
#	endif
