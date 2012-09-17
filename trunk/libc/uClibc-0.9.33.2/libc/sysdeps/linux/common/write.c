/* vi: set sw=4 ts=4: */
/*
 * write() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

_syscall3(ssize_t, write, int, fd, const __ptr_t, buf, size_t, count)
#ifndef __LINUXTHREADS_OLD__
libc_hidden_def(write)
#else
libc_hidden_weak(write)
strong_alias(write,__libc_write)
#endif

#if 0
/* Stupid libgcc.a from gcc 2.95.x uses __write in pure.o
 * which is a blatant GNU libc-ism... */
strong_alias(write,__write)
#endif
