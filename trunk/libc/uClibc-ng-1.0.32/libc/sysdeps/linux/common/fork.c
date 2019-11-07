/*
 * fork() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __ARCH_USE_MMU__
# include <unistd.h>
extern __typeof(fork) __libc_fork;
# if defined __NR_fork
#  include <cancel.h>
#  define __NR___libc_fork __NR_fork
_syscall0(pid_t, fork)

# elif defined __NR_clone  && !defined __NR_fork
#  include <sys/types.h>
#  include <signal.h>
#  include <stddef.h>
pid_t fork(void)
{
	pid_t pid = INLINE_SYSCALL(clone, 4, SIGCHLD, NULL, NULL, NULL);

	if (pid < 0)
		return -1;

	return pid;
}

# endif
# ifdef __UCLIBC_HAS_THREADS__
strong_alias(fork,__libc_fork)
libc_hidden_weak(fork)
# else
libc_hidden_def(fork)
# endif

#endif
