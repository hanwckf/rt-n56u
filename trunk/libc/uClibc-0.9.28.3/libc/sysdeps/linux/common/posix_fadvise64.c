/* vi: set sw=4 ts=4: */
/*
 * posix_fadvise64() for uClibc
 * http://www.opengroup.org/onlinepubs/009695399/functions/posix_fadvise.html
 *
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
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
#ifdef __NR_fadvise64_64
#define __NR___syscall_fadvise64_64 __NR_fadvise64_64

/* 64 bit implementation is cake ... or more like pie ... */
#if __WORDSIZE == 64
_syscall4(int, __syscall_fadvise64_64, int, fd, __off64_t, offset,
          __off64_t, len, int, advice);
int __libc_posix_fadvise64(int fd, __off64_t offset, __off64_t len, int advice)
{
	return (__syscall_fadvise64_64(fd, offset, len, advice));
}
weak_alias(__libc_posix_fadvise64, posix_fadvise64);

/* 32 bit implementation is kind of a pita */
#elif __WORDSIZE == 32

#ifdef _syscall6 /* workaround until everyone has _syscall6() */
_syscall6(int, __syscall_fadvise64_64, int, fd,
          unsigned long, high_offset, unsigned long, low_offset,
          unsigned long, high_len, unsigned long, low_len,
          int, advice);
int __libc_posix_fadvise64(int fd, __off64_t offset, __off64_t len, int advice)
{
	return (__syscall_fadvise64_64(fd,
	        __LONG_LONG_PAIR(offset >> 32, offset &  0xffffffff),
	        __LONG_LONG_PAIR(len >> 32, len & 0xffffffff),
	        advice));
}
weak_alias(__libc_posix_fadvise64, posix_fadvise64);
#else
#warning _syscall6 has not been defined for your machine :(
#endif /* _syscall6 */

#else
#error your machine is neither 32 bit or 64 bit ... it must be magical
#endif

#elif !defined __NR_fadvise64
/* This is declared as a weak alias in posix_fadvice.c if __NR_fadvise64
 * is defined.
 */
int posix_fadvise64(int fd, __off64_t offset, __off64_t len, int advice)
{
	__set_errno(ENOSYS);
	return -1;
}
#endif /* __NR_fadvise64_64 */
#endif /* __UCLIBC_HAS_LFS__ */
