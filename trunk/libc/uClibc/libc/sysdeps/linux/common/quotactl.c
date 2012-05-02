/* vi: set sw=4 ts=4: */
/*
 * quotactl() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/quota.h>
_syscall4(int, quotactl, int, cmd, const char *, special,
		  int, id, caddr_t, addr);
