/*
 * quotactl() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __USE_BSD
#include <sys/quota.h>
_syscall4(int, quotactl, int, cmd, const char *, special,
		  int, id, caddr_t, addr)
#endif
