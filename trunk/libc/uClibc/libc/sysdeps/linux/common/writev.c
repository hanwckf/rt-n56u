/* vi: set sw=4 ts=4: */
/*
 * writev() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/uio.h>
_syscall3(ssize_t, writev, int, filedes, const struct iovec *, vector,
		  int, count);
