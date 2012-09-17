/* vi: set sw=4 ts=4: */
/*
 * vmsplice() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <fcntl.h>

#ifdef __NR_vmsplice
_syscall4(ssize_t, vmsplice, int, __fdout, const struct iovec *, __iov,
	size_t, __count, unsigned int, __flags)
#endif
