/* vi: set sw=4 ts=4: */
/*
 * sched_get_priority_min() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sched.h>
_syscall1(int, sched_get_priority_min, int, policy)
