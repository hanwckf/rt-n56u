/* vi: set sw=4 ts=4: */
/*
 * posix_fadvise() for uClibc
 * http://www.opengroup.org/onlinepubs/009695399/functions/posix_fadvise.html
 *
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <fcntl.h>

#ifdef __NR_fadvise64
#define __NR___syscall_fadvise64 __NR_fadvise64
_syscall4(int, __syscall_fadvise64, int, fd, off_t, offset,
          off_t, len, int, advice);
int __libc_posix_fadvise(int fd, off_t offset, off_t len, int advice)
{
	return (__syscall_fadvise64(fd, offset, len, advice));
}
weak_alias(__libc_posix_fadvise, posix_fadvise);

#if defined __UCLIBC_HAS_LFS__ && (!defined __NR_fadvise64_64 || !defined _syscall6)
weak_alias(__libc_posix_fadvise, posix_fadvise64);
#endif

#else
int posix_fadvise(int fd, off_t offset, off_t len, int advice)
{
	__set_errno(ENOSYS);
	return -1;
}
#endif
