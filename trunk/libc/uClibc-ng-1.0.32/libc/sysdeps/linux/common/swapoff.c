/*
 * swapoff() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifdef __NR_swapoff

#include <sys/swap.h>
_syscall1(int, swapoff, const char *, path)

#endif
