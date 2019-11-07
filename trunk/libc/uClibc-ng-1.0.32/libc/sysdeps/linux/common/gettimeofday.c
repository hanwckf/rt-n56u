/*
 * gettimeofday() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>

_syscall2(int, gettimeofday, struct timeval *, tv, __timezone_ptr_t, tz)
libc_hidden_def(gettimeofday)
