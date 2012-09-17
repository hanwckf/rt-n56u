/*
 * readlinkat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_readlinkat
_syscall4(ssize_t, readlinkat, int, fd, const char *, path, char *, buf, size_t, len)
#else
/* should add emulation with readlink() and /proc/self/fd/ ... */
#endif
