/* vi: set sw=4 ts=4: */
/*
 * iopl() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#if defined __ARCH_USE_MMU__ && defined __NR_iopl
/* psm: can't #include <sys/io.h>, some archs miss it */
extern int iopl(int __level) __THROW;
_syscall1(int, iopl, int, level)
#endif
