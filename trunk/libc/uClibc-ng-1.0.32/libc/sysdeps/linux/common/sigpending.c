/*
 * sigpending() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __USE_POSIX
#include <signal.h>
#undef sigpending

#ifdef __NR_rt_sigpending
# define __NR___rt_sigpending __NR_rt_sigpending
static __inline__ _syscall2(int, __rt_sigpending, sigset_t *, set, size_t, size)

int sigpending(sigset_t * set)
{
	return __rt_sigpending(set, __SYSCALL_SIGSET_T_SIZE);
}
#else
_syscall1(int, sigpending, sigset_t *, set)
#endif
#endif
