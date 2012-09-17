/*
 * unlinkat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_unlinkat
_syscall3(int, unlinkat, int, fd, const char *, file, int, flag)
#else
/* should add emulation with unlink() and /proc/self/fd/ ... */
#endif
