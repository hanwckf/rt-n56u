/* vi: set sw=4 ts=4: */
/*
 * __syscall_sigaction() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"

#ifndef __NR_rt_sigaction
#define __NR___syscall_sigaction __NR_sigaction
#include <signal.h>
#undef sigaction
_syscall3(int, __syscall_sigaction, int, signum, const struct sigaction *,
		  act, struct sigaction *, oldact);
#endif

