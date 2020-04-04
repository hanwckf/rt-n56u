/*
 * swapon() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifdef __NR_swapon

#include <sys/swap.h>
_syscall2(int, swapon, const char *, path, int, swapflags)

#endif
