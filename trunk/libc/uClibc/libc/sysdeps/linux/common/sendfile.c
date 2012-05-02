/* vi: set sw=4 ts=4: */
/*
 * sendfile() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#include <sys/sendfile.h>
_syscall4(ssize_t, sendfile, int, out_fd, int, in_fd, __off_t *, offset,
		  size_t, count);
