/*
 * times() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/times.h>

_syscall_noerr1(clock_t, times, struct tms *, buf)
libc_hidden_def(times)
