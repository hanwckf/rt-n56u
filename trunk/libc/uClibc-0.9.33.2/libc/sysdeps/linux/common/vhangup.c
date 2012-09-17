/* vi: set sw=4 ts=4: */
/*
 * vhangup() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#if defined __USE_BSD || (defined __USE_XOPEN && !defined __USE_UNIX98)
_syscall0(int, vhangup)
#endif
