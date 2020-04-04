/*
 * fdatasync() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if !defined __NR_fdatasync && defined __NR_osf_fdatasync
# define __NR_fdatasync __NR_osf_fdatasync
#endif

#ifdef __NR_fdatasync
# include <unistd.h>
# include <cancel.h>

# define __NR___fdatasync_nocancel __NR_fdatasync
static _syscall1(int, __NC(fdatasync), int, fd)

CANCELLABLE_SYSCALL(int, fdatasync, (int fd), (fd))
#endif
