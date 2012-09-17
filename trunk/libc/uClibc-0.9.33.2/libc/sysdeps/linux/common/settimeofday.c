/* vi: set sw=4 ts=4: */
/*
 * settimeofday() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>

#ifdef __USE_BSD


_syscall2(int, settimeofday, const struct timeval *, tv,
		  const struct timezone *, tz)
libc_hidden_def(settimeofday)
#endif
