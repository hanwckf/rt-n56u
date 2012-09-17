/*
 * openat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define openat64 __xx_openat
#include <sys/syscall.h>
#include <fcntl.h>
#undef openat64

#ifdef __UCLIBC_HAS_LFS__

#ifdef __NR_openat
/* The openat() prototype is varargs based, but we don't care about that
 * here, so need to provide our own dedicated signature.
 */
extern int openat64(int fd, const char *file, int oflag, mode_t mode);
libc_hidden_proto(openat64)

int openat64(int fd, const char *file, int oflag, mode_t mode)
{
	return openat(fd, file, oflag | O_LARGEFILE, mode);
}
libc_hidden_def(openat64)
#else
/* should add emulation with open() and /proc/self/fd/ ... */
#endif

#endif
