/* vi: set sw=4 ts=4: */
/*
 * delete_module() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#ifdef __NR_delete_module
_syscall1(int, delete_module, const char *, name);
#else
int delete_module(const char *name)
{
	__set_errno(ENOSYS);
	return -1;
}
#endif
