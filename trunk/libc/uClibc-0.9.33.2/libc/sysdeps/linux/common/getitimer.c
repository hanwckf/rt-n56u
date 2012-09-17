/* vi: set sw=4 ts=4: */
/*
 * getitimer() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>
_syscall2(int, getitimer, __itimer_which_t, which, struct itimerval *, value)
