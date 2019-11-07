/*
 * bdflush() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/kdaemon.h>

#ifdef __NR_bdflush
_syscall2(int, bdflush, int, __func, long int, __data)
#endif
