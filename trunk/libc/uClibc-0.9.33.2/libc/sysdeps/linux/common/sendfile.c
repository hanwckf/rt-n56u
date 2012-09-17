/* vi: set sw=4 ts=4: */
/*
 * sendfile() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <sys/sendfile.h>

#ifdef __NR_sendfile

_syscall4(ssize_t, sendfile, int, out_fd, int, in_fd, __off_t *, offset,
		  size_t, count)

#if ! defined __NR_sendfile64 && defined __UCLIBC_HAS_LFS__
strong_alias(sendfile,sendfile64)
#endif

#endif /* __NR_sendfile */
