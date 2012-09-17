/* vi: set sw=4 ts=4: */
/*
 * ioperm() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __ARCH_USE_MMU__ && defined __NR_ioperm

/* psm: can't #include <sys/io.h>, some archs miss it */
extern int ioperm(unsigned long __from, unsigned long __num, int __turn_on) __THROW;
/* Not needed internally;
libc_hidden_proto(ioperm)
*/
_syscall3(int, ioperm, unsigned long, from, unsigned long, num, int, turn_on)
/*libc_hidden_def(ioperm)*/

#endif
