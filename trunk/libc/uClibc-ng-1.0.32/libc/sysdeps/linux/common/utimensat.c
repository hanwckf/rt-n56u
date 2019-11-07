/*
 * utimensat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>

#ifdef __NR_utimensat
_syscall4(int, utimensat, int, fd, const char *, path, const struct timespec *, times, int, flags)
libc_hidden_def(utimensat)
#else
/* should add emulation with utimens() and /proc/self/fd/ ... */
#endif

