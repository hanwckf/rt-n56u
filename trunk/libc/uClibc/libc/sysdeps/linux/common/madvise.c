/* vi: set sw=4 ts=4: */
/*
 * madvise() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#ifdef __NR_madvise
_syscall3(int, madvise, void *, __addr, size_t, __len, int, __advice);
#endif
