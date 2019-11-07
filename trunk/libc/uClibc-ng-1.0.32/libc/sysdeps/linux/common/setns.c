/*
 * setns() for uClibc
 *
 * Copyright (C) 2015 Bernhard Reutner-Fischer <uclibc@uclibc.org>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sched.h>

#ifdef __NR_setns
_syscall2(int, setns, int, fd, int, nstype)
#endif
