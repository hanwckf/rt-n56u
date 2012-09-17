/* vi: set sw=4 ts=4: */
/*
 * posix_fallocate() for uClibc
 * http://www.opengroup.org/onlinepubs/009695399/functions/posix_fallocate.html
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <fcntl.h>

#ifndef __USE_FILE_OFFSET64

#ifdef __NR_fallocate
int posix_fallocate(int fd, off_t offset, off_t len)
{
	INTERNAL_SYSCALL_DECL(err);
	int ret = (int) (INTERNAL_SYSCALL(fallocate, err, 6, fd, 0,
					__LONG_LONG_PAIR (offset >> 31, offset),
					__LONG_LONG_PAIR (len >> 31, len)));
    if (INTERNAL_SYSCALL_ERROR_P (ret, err))
      return INTERNAL_SYSCALL_ERRNO (ret, err);
    return 0;
}

#elif defined __UCLIBC_HAS_STUBS__
int posix_fallocate(int fd attribute_unused, off_t offset attribute_unused, off_t len attribute_unused)
{
	return ENOSYS;
}
#endif

#endif /* !__USE_FILE_OFFSET64 */
