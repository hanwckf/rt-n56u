/* vi: set sw=4 ts=4: */
/*
 * __syscall_ipc() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"

#ifdef __NR_ipc
#define __NR___syscall_ipc __NR_ipc
_syscall5(int, __syscall_ipc, unsigned int, call, int, first, int, second, int,
		  third, void *, ptr);
#endif
