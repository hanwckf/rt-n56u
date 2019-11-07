/*
 * posix_fallocate() for uClibc
 * http://www.opengroup.org/onlinepubs/9699919799/functions/posix_fallocate.html
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <fcntl.h>
#include <bits/kernel-features.h>
#include <stdint.h>
#include <errno.h>

#if defined __NR_fallocate
# if __WORDSIZE == 64
/* Can use normal posix_fallocate() */
# elif __WORDSIZE == 32
extern __typeof(fallocate64) __libc_fallocate64 attribute_hidden;
int posix_fallocate64(int fd, __off64_t offset, __off64_t len)
{
	if (__libc_fallocate64(fd, 0, offset, len))
		return errno;
	return 0;
}
# else
#  error your machine is neither 32 bit or 64 bit ... it must be magical
# endif
#endif
