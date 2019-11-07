/*
 * fsync() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <cancel.h>

#define __NR___fsync_nocancel __NR_fsync
static _syscall1(int, __NC(fsync), int, fd)

CANCELLABLE_SYSCALL(int, fsync, (int fd), (fd))
