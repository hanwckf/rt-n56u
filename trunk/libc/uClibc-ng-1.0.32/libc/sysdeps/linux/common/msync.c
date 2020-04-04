/*
 * msync() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __NR_msync && defined __ARCH_USE_MMU__
# include <sys/mman.h>
# include <cancel.h>

# define __NR___msync_nocancel __NR_msync
static _syscall3(int, __NC(msync), void *, addr, size_t, length, int, flags)

CANCELLABLE_SYSCALL(int, msync, (void *addr, size_t length, int flags),
		    (addr, length, flags))
#endif
