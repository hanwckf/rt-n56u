/*
 * fchdir() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
_syscall1(int, fchdir, int, fd)
libc_hidden_def(fchdir)
#endif
