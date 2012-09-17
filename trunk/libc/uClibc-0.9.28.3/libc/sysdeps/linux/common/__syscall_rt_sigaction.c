/* vi: set sw=4 ts=4: */
/*
 * __syscall_rt_sigaction() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"

#ifdef __NR_rt_sigaction
#include <signal.h>

#define __NR___syscall_rt_sigaction __NR_rt_sigaction
#undef sigaction
_syscall4(int, __syscall_rt_sigaction, int, signum,
		  const struct sigaction *, act, struct sigaction *, oldact,
		  size_t, size);

#endif
