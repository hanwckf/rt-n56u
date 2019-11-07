/*
 * sysfs() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* libc isn't really supposed to export this */
#if 0
#include <sys/syscall.h>

#if defined __USE_SVID
_syscall3(int, sysfs, int, option, unsigned int, index, char, addr)
#endif
#endif
