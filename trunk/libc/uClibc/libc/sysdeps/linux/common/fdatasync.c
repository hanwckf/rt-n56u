/* vi: set sw=4 ts=4: */
/*
 * fdatasync() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#if defined (__alpha__)
#undef  __NR_fdatasync
#define __NR_fdatasync __NR_osf_fdatasync
#endif

_syscall1(int, fdatasync, int, fd);

