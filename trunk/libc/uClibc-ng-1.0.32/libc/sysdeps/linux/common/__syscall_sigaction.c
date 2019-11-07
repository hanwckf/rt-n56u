/*
 * __syscall_sigaction() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifndef __NR_rt_sigaction
#define __NR___syscall_sigaction __NR_sigaction
#include <signal.h>
_syscall3(int, __syscall_sigaction, int, signum, const struct sigaction *,
	  act, struct sigaction *, oldact)
#endif

