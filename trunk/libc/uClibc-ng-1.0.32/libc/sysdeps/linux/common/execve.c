/*
 * execve() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>

_syscall3(int, execve, const char *, filename,
		  char *const *, argv, char *const *, envp)
libc_hidden_def(execve)
