/* vi: set sw=4 ts=4: */
/*
 * fork() for Xtensa uClibc
 *
 * Copyright (C) 2007 Tensilica Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <sys/syscall.h>
#define _SIGNAL_H
#include <bits/signum.h>

/* Xtensa doesn't provide a 'fork' system call, so we use 'clone'.  */

extern __typeof(fork) __libc_fork;

libc_hidden_proto(fork)
pid_t __libc_fork(void)
{
	return (pid_t) INLINE_SYSCALL(clone, 2, SIGCHLD, 0);
}
weak_alias(__libc_fork, fork)
libc_hidden_weak(fork)
