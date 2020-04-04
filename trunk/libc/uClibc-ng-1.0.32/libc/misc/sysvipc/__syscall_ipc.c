/*
 * __syscall_ipc() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifdef __NR_ipc
#define __NR___syscall_ipc __NR_ipc
#include "ipc.h"
_syscall6(int, __syscall_ipc, unsigned int, call, long, first, long, second, long,
		  third, void *, ptr, void *, fifth)
#endif
