/* vi: set sw=4 ts=4: */
/*
 * mlock() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/mman.h>
#	if defined __ARCH_HAS_MMU__ && defined __NR_mlock
_syscall2(int, mlock, const void *, addr, size_t, len);
#	endif
