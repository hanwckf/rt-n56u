/*
 * fallocate() for uClibc - based off posix_fallocate() by Erik Andersen
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
/* Can use normal fallocate() */
# elif __WORDSIZE == 32
extern __typeof(fallocate64) __libc_fallocate64 attribute_hidden;
int attribute_hidden __libc_fallocate64(int fd, int mode, __off64_t offset,
		__off64_t len)
{
	int ret;
	INTERNAL_SYSCALL_DECL(err);
	ret = (int) (INTERNAL_SYSCALL(fallocate, err, 6, fd, mode,
		OFF64_HI_LO (offset), OFF64_HI_LO (len)));
	if (unlikely(INTERNAL_SYSCALL_ERROR_P (ret, err))) {
		__set_errno(INTERNAL_SYSCALL_ERRNO (ret, err));
		ret = -1;
	}
	return ret;
}

#  if defined __UCLIBC_LINUX_SPECIFIC__ && defined __USE_GNU
strong_alias(__libc_fallocate64,fallocate64)
#  endif

# else
# error your machine is neither 32 bit or 64 bit ... it must be magical
# endif
#endif
