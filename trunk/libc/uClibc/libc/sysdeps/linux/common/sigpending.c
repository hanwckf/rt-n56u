/* vi: set sw=4 ts=4: */
/*
 * sigpending() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <signal.h>
#undef sigpending

#ifdef __NR_rt_sigpending
#define __NR___rt_sigpending __NR_rt_sigpending
_syscall2(int, __rt_sigpending, sigset_t *, set, size_t, size);

int sigpending(sigset_t * set)
{
	return __rt_sigpending(set, _NSIG / 8);
}

#else
_syscall1(int, sigpending, sigset_t *, set);
#endif
