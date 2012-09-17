/* vi: set sw=4 ts=4: */
/*
 * sched_yield() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sched.h>
_syscall0(int, sched_yield)
