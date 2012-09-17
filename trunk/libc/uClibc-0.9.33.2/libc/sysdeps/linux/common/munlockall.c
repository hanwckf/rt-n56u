/* vi: set sw=4 ts=4: */
/*
 * munlockall() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __NR_munlockall && defined __ARCH_USE_MMU__
#include <sys/mman.h>

_syscall0(int, munlockall)
#endif
