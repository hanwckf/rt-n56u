/* vi: set sw=4 ts=4: */
/*
 * posix_fadvise() for Xtensa uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 * Copyright (C) 2007 Tensilica Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <fcntl.h>

int posix_fadvise (int fd, off_t offset, off_t len, int advice)
{
#ifdef __NR_fadvise64_64
	INTERNAL_SYSCALL_DECL (err);
	int ret = INTERNAL_SYSCALL (fadvise64_64, err, 6, fd, advice,
								__LONG_LONG_PAIR ((long) (offset >> 31),
												  (long) offset),
								__LONG_LONG_PAIR ((long) (len >> 31),
												  (long) len));
	if (!INTERNAL_SYSCALL_ERROR_P (ret, err))
		return 0;
	return INTERNAL_SYSCALL_ERRNO (ret, err);
#else
	return ENOSYS;
#endif
}
