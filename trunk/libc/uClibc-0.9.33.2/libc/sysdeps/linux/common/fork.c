/* vi: set sw=4 ts=4: */
/*
 * fork() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __ARCH_USE_MMU__

#ifdef __NR_fork
#define __NR___libc_fork __NR_fork
extern __typeof(fork) __libc_fork;
_syscall0(pid_t, __libc_fork)
weak_alias(__libc_fork,fork)
libc_hidden_weak(fork)
#endif

#endif
