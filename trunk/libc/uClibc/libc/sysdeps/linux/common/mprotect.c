/* vi: set sw=4 ts=4: */
/*
 * mprotect() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/mman.h>
_syscall3(int, mprotect, void *, addr, size_t, len, int, prot);
