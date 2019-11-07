/*
 * prctl() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
/* psm: including sys/prctl.h would depend on kernel headers */

#ifdef __NR_prctl
int prctl (int, long, long, long, long);
_syscall5(int, prctl, int, option, long, _prctl_a2, long, _prctl_a3,
		long, _prctl_a4, long, _prctl_a5)
#endif
