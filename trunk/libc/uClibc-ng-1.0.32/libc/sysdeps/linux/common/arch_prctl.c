/*
 * arch_prctl() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifdef __NR_arch_prctl
int arch_prctl(int code, unsigned long addr);
_syscall2(int, arch_prctl, int, code, unsigned long, addr)
#endif
