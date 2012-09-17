/*
 * renameat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <stdio.h>

#ifdef __NR_renameat
_syscall4(int, renameat, int, oldfd, const char *, old, int, newfd, const char *, new)
#else
/* should add emulation with rename() and /proc/self/fd/ ... */
#endif
