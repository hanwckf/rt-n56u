/*
 * fallocate() for uClibc - Based off of posix_fallocate() by Erik Andersen
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
extern __typeof(fallocate) __libc_fallocate attribute_hidden;
int attribute_hidden __libc_fallocate(int fd, int mode, __off_t offset, __off_t len)
{
# if __WORDSIZE == 32
	return fallocate64(fd, mode, offset, len);
# elif __WORDSIZE == 64
	int ret;
	INTERNAL_SYSCALL_DECL(err);
	ret = (int) (INTERNAL_SYSCALL(fallocate, err, 4, fd, mode, offset, len));
	if (unlikely(INTERNAL_SYSCALL_ERROR_P (ret, err))) {
		__set_errno(INTERNAL_SYSCALL_ERRNO (ret, err));
		ret = -1;
	}
	return ret;
# else
# error your machine is neither 32 bit or 64 bit ... it must be magical
# endif
}

# if defined __UCLIBC_LINUX_SPECIFIC__ && defined __USE_GNU
strong_alias(__libc_fallocate,fallocate)
#  if __WORDSIZE == 64
strong_alias(__libc_fallocate,fallocate64)
#  endif
# endif
#endif
