/* vi: set sw=4 ts=4: */
/*
 * getpid() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#if defined (__alpha__)
#define __NR_getpid     __NR_getxpid
#endif
#define __NR___libc_getpid __NR_getpid
_syscall0(pid_t, __libc_getpid);
weak_alias(__libc_getpid, getpid);
weak_alias(__libc_getpid, __getpid);
