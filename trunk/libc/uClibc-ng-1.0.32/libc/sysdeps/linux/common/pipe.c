/*
 * pipe() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>


#if defined __NR_pipe2 && !defined __NR_pipe
int pipe(int filedes[2])
{
	return pipe2(filedes, 0);
}
/* If both are defined then use the pipe syscall */
#else
_syscall1(int, pipe, int *, filedes)
#endif
libc_hidden_def(pipe)
