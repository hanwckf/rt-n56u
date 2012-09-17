/* vi: set sw=4 ts=4: */
/*
 * sysinfo() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/sysinfo.h>
_syscall1(int, sysinfo, struct sysinfo *, info)
