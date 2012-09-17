/* vi: set sw=4 ts=4: */
/*
 * posix_fallocate64() for uClibc
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

#ifdef __UCLIBC_HAS_LFS__

#ifdef __NR_fallocate
int posix_fallocate64(int fd, __off64_t offset, __off64_t len)
{
  if (len != (off_t) len)
    return EOVERFLOW;
  INTERNAL_SYSCALL_DECL (err);
    int ret = INTERNAL_SYSCALL (fallocate, err, 6, fd, 0,
								__LONG_LONG_PAIR ((long int) (offset >> 32),
												  (long int) offset),
								__LONG_LONG_PAIR ((long int) (len >> 32),
												  (long int) len));
  if (!INTERNAL_SYSCALL_ERROR_P (ret, err))
    return 0;
  return INTERNAL_SYSCALL_ERRNO (ret, err);
}

#elif defined __UCLIBC_HAS_STUBS__
int posix_fallocate64(int fd attribute_unused, off64_t offset attribute_unused, off64_t len attribute_unused)
{
	return ENOSYS;
}
#endif /* __NR_fallocate */

#endif /* __UCLIBC_HAS_LFS__ */
