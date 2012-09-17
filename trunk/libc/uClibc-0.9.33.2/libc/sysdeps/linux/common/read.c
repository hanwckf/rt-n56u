/* vi: set sw=4 ts=4: */
/*
 * read() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

_syscall3(ssize_t, read, int, fd, __ptr_t, buf, size_t, count)
#ifndef __LINUXTHREADS_OLD__
libc_hidden_def(read)
#else
libc_hidden_weak(read)
strong_alias(read,__libc_read)
#endif
