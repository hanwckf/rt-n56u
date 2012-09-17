/*
 * linkat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_linkat
_syscall5(int, linkat, int, fromfd, const char *, from, int, tofd, const char *, to, int, flags)
#else
/* should add emulation with link() and /proc/self/fd/ ... */
#endif
