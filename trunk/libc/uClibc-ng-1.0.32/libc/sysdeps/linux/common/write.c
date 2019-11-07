/*
 * write() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <cancel.h>

#define __NR___write_nocancel __NR_write
_syscall3(ssize_t, __NC(write), int, fd, const void *, buf, size_t, count)

CANCELLABLE_SYSCALL(ssize_t, write, (int fd, const void *buf, size_t count),
		    (fd, buf, count))
lt_libc_hidden(write)
