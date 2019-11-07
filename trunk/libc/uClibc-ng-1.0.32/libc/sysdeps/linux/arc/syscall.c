/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>

extern long syscall(long int sysnum, long a, long b, long c, long d, long e, long f);

long syscall(long int sysnum, long a, long b, long c, long d, long e, long f)
{
	return INLINE_SYSCALL_NCS(sysnum, 6, a, b, c, d, e, f);
}
