/* vi: set sw=4 ts=4: */
/*
 * nanosleep() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <time.h>

#define __NR___libc_nanosleep __NR_nanosleep
_syscall2(int, __libc_nanosleep, const struct timespec *, req,
		  struct timespec *, rem);
weak_alias(__libc_nanosleep, nanosleep);
