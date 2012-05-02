/* vi: set sw=4 ts=4: */
/*
 * sigsuspend() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <signal.h>
#undef sigsuspend

#ifdef __NR_rt_sigsuspend
#define __NR___rt_sigsuspend __NR_rt_sigsuspend
_syscall2(int, __rt_sigsuspend, const sigset_t *, mask, size_t, size);

int sigsuspend(const sigset_t * mask)
{
	return __rt_sigsuspend(mask, _NSIG / 8);
}

#else

#define __NR___sigsuspend __NR_sigsuspend
_syscall3(int, __sigsuspend, int, a, unsigned long int, b,
		  unsigned long int, c);

int sigsuspend(const sigset_t * set)
{
	return __sigsuspend(0, 0, set->__val[0]);
}

#endif
