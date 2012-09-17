/* vi: set sw=4 ts=4: */
/*
 * posix_fadvise64() for Xtensa uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 * Copyright (C) 2007 Tensilica Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <unistd.h>
#include <errno.h>
#include <endian.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <fcntl.h>

#ifdef __UCLIBC_HAS_LFS__

int posix_fadvise64 (int fd, __off64_t offset, __off64_t len, int advice)
{
#ifdef __NR_fadvise64_64
	INTERNAL_SYSCALL_DECL (err);
	int ret = INTERNAL_SYSCALL (fadvise64_64, err, 6, fd, advice,
								__LONG_LONG_PAIR ((long) (offset >> 32),
												  (long) offset),
								__LONG_LONG_PAIR ((long) (len >> 32),
												  (long) len));
	if (!INTERNAL_SYSCALL_ERROR_P (ret, err))
		return 0;
	return INTERNAL_SYSCALL_ERRNO (ret, err);
#else
	return ENOSYS;
#endif
}

#endif /* __UCLIBC_HAS_LFS__ */
