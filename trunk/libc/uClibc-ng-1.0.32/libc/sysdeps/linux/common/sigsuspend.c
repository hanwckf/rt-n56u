/*
 * sigsuspend() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifdef __USE_POSIX
#include <signal.h>
#include <cancel.h>

int __NC(sigsuspend)(const sigset_t *set)
{
#ifdef __NR_rt_sigsuspend
	return INLINE_SYSCALL(rt_sigsuspend, 2, set, __SYSCALL_SIGSET_T_SIZE);
#else
	return INLINE_SYSCALL(sigsuspend, 3, 0, 0, set->__val[0]);
#endif
}
CANCELLABLE_SYSCALL(int, sigsuspend, (const sigset_t *set), (set))
lt_libc_hidden(sigsuspend)
#endif
