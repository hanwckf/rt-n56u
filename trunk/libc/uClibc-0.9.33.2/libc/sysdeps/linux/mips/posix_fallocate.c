/* vi: set sw=4 ts=4: */
/*
 * posix_fallocate() for MIPS uClibc
 * http://www.opengroup.org/onlinepubs/009695399/functions/posix_fallocate.html
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
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

int posix_fallocate(int fd, off_t offset, off_t len)
{
#ifdef __NR_fallocate
	INTERNAL_SYSCALL_DECL(err);
# if _MIPS_SIM == _ABIO32
	int ret = INTERNAL_SYSCALL(fallocate, err, 6, fd, 0,
		__LONG_LONG_PAIR ((long) (offset >> 31), (long) offset),
		__LONG_LONG_PAIR ((long) (len >> 31), (long) len));
# else /* N32 || N64 */
	int ret = INTERNAL_SYSCALL(fallocate, err, 4, fd, offset, len);
# endif
	if (INTERNAL_SYSCALL_ERROR_P (ret, err))
		return INTERNAL_SYSCALL_ERRNO (ret, err);
	return 0;
#else
	return ENOSYS;
#endif
}
