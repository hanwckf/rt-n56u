/* vi: set sw=4 ts=4: */
/*
 * read() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>

#define __NR___libc_read __NR_read
_syscall3(ssize_t, __libc_read, int, fd, __ptr_t, buf, size_t, count);
weak_alias(__libc_read, read);
