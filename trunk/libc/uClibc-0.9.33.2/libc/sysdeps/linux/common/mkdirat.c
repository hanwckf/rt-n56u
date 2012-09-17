/*
 * mkdirat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>

#ifdef __NR_mkdirat
_syscall3(int, mkdirat, int, fd, const char *, path, mode_t, mode)
#else
/* should add emulation with mkdir() and /proc/self/fd/ ... */
#endif
