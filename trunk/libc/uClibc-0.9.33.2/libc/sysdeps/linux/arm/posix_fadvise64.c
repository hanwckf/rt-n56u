/* vi: set sw=4 ts=4: */
/*
 * posix_fadvise64() for ARM uClibc
 * http://www.opengroup.org/onlinepubs/009695399/functions/posix_fadvise.html
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

#if defined __NR_arm_fadvise64_64

/* Was named __libc_posix_fadvise64 for some inexplicable reason.
** google says only uclibc has *__libc*_posix_fadviseXXX,
** so it cannot be compat with anything.
**
** Remove this comment and one at the end after 0.9.31
*/

/* This is for the ARM version of fadvise64_64 which swaps the params
 * about to avoid having ABI compat issues
 */
#define __NR___syscall_arm_fadvise64_64 __NR_arm_fadvise64_64
int posix_fadvise64(int fd, __off64_t offset, __off64_t len, int advise)
{
  INTERNAL_SYSCALL_DECL (err);
  int ret = INTERNAL_SYSCALL (arm_fadvise64_64, err, 6, fd, advise,
                              __LONG_LONG_PAIR ((long)(offset >> 32), (long)offset),
                              __LONG_LONG_PAIR ((long)(len >> 32), (long)len));
  if (!INTERNAL_SYSCALL_ERROR_P (ret, err))
    return 0;
  if (INTERNAL_SYSCALL_ERRNO (ret, err) != ENOSYS)
   return INTERNAL_SYSCALL_ERRNO (ret, err);
  return 0;
}

/* weak_alias(__libc_posix_fadvise64, posix_fadvise64); */

#elif defined __UCLIBC_HAS_STUBS__

int posix_fadvise64(int fd, __off64_t offset, __off64_t len, int advise)
{
	return ENOSYS;
}

#endif

#endif
