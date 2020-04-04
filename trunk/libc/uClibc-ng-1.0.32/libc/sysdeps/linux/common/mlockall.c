/*
 * mlockall() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/mman.h>
#if defined __ARCH_USE_MMU__ && defined __NR_mlockall
_syscall1(int, mlockall, int, flags)
#endif
