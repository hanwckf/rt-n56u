/* vi: set sw=4 ts=4: */
/*
 * gettimeofday() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>

#ifdef __USE_BSD
_syscall2(int, gettimeofday, struct timeval *, tv, struct timezone *, tz)
#else
_syscall2(int, gettimeofday, struct timeval *, tv, void *, tz)
#endif
libc_hidden_def(gettimeofday)
