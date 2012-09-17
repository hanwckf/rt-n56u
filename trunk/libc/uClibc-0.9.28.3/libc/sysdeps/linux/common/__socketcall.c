/* vi: set sw=4 ts=4: */
/*
 * __socketcall() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#ifdef __NR_socketcall
#define __NR___socketcall __NR_socketcall
_syscall2(int, __socketcall, int, call, unsigned long *, args);
#endif
