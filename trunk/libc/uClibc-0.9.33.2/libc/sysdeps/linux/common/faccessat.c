/*
 * faccessat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_faccessat
_syscall4(int, faccessat, int, fd, const char *, file, int, type, int, flag)
#else
/* should add emulation with faccess() and /proc/self/fd/ ... */
#endif
