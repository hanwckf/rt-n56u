/* vi: set sw=4 ts=4: */
/*
 * prctl() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <stdarg.h>
/* psm: including sys/prctl.h would depend on kernel headers */

#ifdef __NR_prctl
extern int prctl (int, long, long, long, long);
_syscall5(int, prctl, int, option, long, _a2, long, _a3, long, _a4, long, _a5)
#endif
