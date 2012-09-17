/* vi: set sw=4 ts=4: */
/*
 * posix_fadvise() for uClibc
 * http://www.opengroup.org/onlinepubs/009695399/functions/posix_fadvise.html
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <fcntl.h>

#ifdef __NR_fadvise64
#define __NR_posix_fadvise __NR_fadvise64
int posix_fadvise(int fd, off_t offset, off_t len, int advice)
{
	INTERNAL_SYSCALL_DECL(err);
	int ret = (int) (INTERNAL_SYSCALL(posix_fadvise, err, 6, fd, 0,
	 __LONG_LONG_PAIR (offset >> 31, offset), len, advice));
    if (INTERNAL_SYSCALL_ERROR_P (ret, err))
      return INTERNAL_SYSCALL_ERRNO (ret, err);
    return 0;
}

#if defined __UCLIBC_HAS_LFS__ && (!defined __NR_fadvise64_64 || !defined _syscall6)
strong_alias(posix_fadvise,posix_fadvise64)
#endif

#else
int posix_fadvise(int fd attribute_unused, off_t offset attribute_unused, off_t len attribute_unused, int advice attribute_unused)
{
#warning This is not correct as far as SUSv3 is concerned.
	return ENOSYS;
}
#endif
