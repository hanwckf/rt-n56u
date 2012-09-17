/* vi: set sw=4 ts=4: */
/*
 * pipe2() for uClibc
 *
 * Copyright (C) 2011 Bernhard Reutner-Fischer <uclibc@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_pipe2
_syscall2(int, pipe2, int *, filedes, int, flags)
libc_hidden_def(pipe2)
#endif
