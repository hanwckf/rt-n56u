/*
 * This file provides the mincore() system call to uClibc.
 * Copyright (C) 20041215 - <solar@gentoo.org>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __NR_mincore && (defined __USE_BSD || defined __USE_SVID)
#include <unistd.h>
#include <sys/mman.h>
_syscall3(int, mincore, void *, start, size_t, length, unsigned char *, vec)
#endif
