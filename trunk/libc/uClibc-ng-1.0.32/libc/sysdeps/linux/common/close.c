/*
 * close() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <cancel.h>

#define __NR___close_nocancel __NR_close
_syscall1(int, __NC(close), int, fd)

#define __NR___close_nocancel_no_status __NR_close
_syscall_noerr1(void, __close_nocancel_no_status, int, fd)

CANCELLABLE_SYSCALL(int, close, (int fd), (fd))
lt_libc_hidden(close)
