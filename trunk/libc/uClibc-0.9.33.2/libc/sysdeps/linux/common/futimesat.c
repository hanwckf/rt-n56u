/*
 * futimesat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>

#ifdef __NR_futimesat
_syscall3(int, futimesat, int, fd, const char *, file, const struct timeval *, tvp)
#else
/* should add emulation with futimes() and /proc/self/fd/ ... */
#endif
