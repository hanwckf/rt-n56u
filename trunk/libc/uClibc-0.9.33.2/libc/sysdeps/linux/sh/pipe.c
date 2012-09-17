/* vi: set sw=4 ts=4: */
/*
 * pipe syscall for uClibc sh
 *
 * Copyright (C) 2001 Lineo, <davidm@lineo.com>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <unistd.h>
#include <syscall.h>


int pipe(int *fd)
{
	long __res, __res2;
	__asm__ __volatile__ (
	"mov	%2,	r3;"
	"mov	%3,	r4;"
	"trapa	%4;"
	"mov    r1, %1;"
	   : "=z" (__res),
	     "=r" ((long) __res2)
	   : "r" ((long) __NR_pipe),
	     "r" ((long) fd),
		 "i" (__SH_SYSCALL_TRAP_BASE + 3)
	   : "cc", "memory", "r1", "r3", "r4");
	if ((unsigned long)(__res) >= (unsigned long)(-125)) {
		int __err = -(__res);
		errno = __err;
		return(-1);
	}
	fd[0] = __res;
	fd[1] = __res2;
	return(0);
}
libc_hidden_def(pipe)
