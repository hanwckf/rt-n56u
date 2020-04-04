/*
 * madvise() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/mman.h>
#ifdef __ARCH_USE_MMU__
#if defined __NR_madvise && defined __USE_BSD
_syscall3(int, madvise, void *, __addr, size_t, __len, int, __advice)
#endif /* __ARCH_USE_MMU__ */
#endif
