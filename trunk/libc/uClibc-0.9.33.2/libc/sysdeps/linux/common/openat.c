/*
 * openat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define openat __xx_openat
#include <sys/syscall.h>
#include <fcntl.h>
#undef openat

#ifdef __NR_openat
/* The openat() prototype is varargs based, but we don't care about that
 * here, so need to provide our own dedicated signature.
 */
extern int openat(int fd, const char *file, int oflag, mode_t mode);
libc_hidden_proto(openat)

_syscall4(int, openat, int, fd, const char *, file, int, oflag, mode_t, mode)
libc_hidden_def(openat)
#else
/* should add emulation with open() and /proc/self/fd/ ... */
#endif
