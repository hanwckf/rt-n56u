/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
/*
 * Based in part on the files
 *		./sysdeps/unix/sysv/linux/pwrite.c,
 *		./sysdeps/unix/sysv/linux/pread.c,
 *		sysdeps/posix/pread.c
 *		sysdeps/posix/pwrite.c
 * from GNU libc 2.2.5, but reworked considerably...
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <endian.h>
#include <bits/wordsize.h>
#include <cancel.h>

#ifdef __NR_pread64
# undef __NR_pread
# define __NR_pread __NR_pread64
#endif
#ifdef __NR_pwrite64
# undef __NR_pwrite
# define __NR_pwrite __NR_pwrite64
#endif

#ifndef MY_PREAD
# ifdef __NR_pread
#  define __NR___syscall_pread __NR_pread
#  if defined(__UCLIBC_SYSCALL_ALIGN_64BIT__)
static _syscall6(ssize_t, __syscall_pread, int, fd, void *, buf,
		 size_t, count, int, dummy, off_t, offset_hi, off_t, offset_lo)
#   define MY_PREAD(fd, buf, count, offset) __syscall_pread(fd, buf, count, 0, OFF_HI_LO(offset))
#   define MY_PREAD64(fd, buf, count, offset) __syscall_pread(fd, buf, count, 0, OFF64_HI_LO(offset))
#  elif __WORDSIZE == 32
static _syscall5(ssize_t, __syscall_pread, int, fd, void *, buf,
		 size_t, count, off_t, offset_hi, off_t, offset_lo)
#   define MY_PREAD(fd, buf, count, offset) __syscall_pread(fd, buf, count, OFF_HI_LO(offset))
#   define MY_PREAD64(fd, buf, count, offset) __syscall_pread(fd, buf, count, OFF64_HI_LO(offset))
#  else
static _syscall4(ssize_t, __syscall_pread, int, fd, void *, buf,
		 size_t, count, off_t, offset)
#   define MY_PREAD(fd, buf, count, offset) __syscall_pread(fd, buf, count, offset)
#   define MY_PREAD64(fd, buf, count, offset) __syscall_pread(fd, buf, count, offset)
#  endif
# endif
#endif

#ifndef MY_PWRITE
# ifdef __NR_pwrite
#  define __NR___syscall_pwrite __NR_pwrite
#  if defined(__UCLIBC_SYSCALL_ALIGN_64BIT__)
static _syscall6(ssize_t, __syscall_pwrite, int, fd, const void *, buf,
		 size_t, count, int, dummy, off_t, offset_hi, off_t, offset_lo)
#   define MY_PWRITE(fd, buf, count, offset) __syscall_pwrite(fd, buf, count, 0, OFF_HI_LO(offset))
#   define MY_PWRITE64(fd, buf, count, offset) __syscall_pwrite(fd, buf, count, 0, OFF64_HI_LO(offset))
#  elif __WORDSIZE == 32
static _syscall5(ssize_t, __syscall_pwrite, int, fd, const void *, buf,
		 size_t, count, off_t, offset_hi, off_t, offset_lo)
#   define MY_PWRITE(fd, buf, count, offset) __syscall_pwrite(fd, buf, count, OFF_HI_LO(offset))
#   define MY_PWRITE64(fd, buf, count, offset) __syscall_pwrite(fd, buf, count, OFF64_HI_LO(offset))
#  else
static _syscall4(ssize_t, __syscall_pwrite, int, fd, const void *, buf,
		 size_t, count, off_t, offset)
#   define MY_PWRITE(fd, buf, count, offset) __syscall_pwrite(fd, buf, count, offset)
#   define MY_PWRITE64(fd, buf, count, offset) __syscall_pwrite(fd, buf, count, offset)
#  endif
# endif
#endif

static ssize_t __NC(pread)(int fd, void *buf, size_t count, off_t offset)
{
	return MY_PREAD(fd, buf, count, offset);
}
CANCELLABLE_SYSCALL(ssize_t, pread, (int fd, void *buf, size_t count, off_t offset),
		    (fd, buf, count, offset))

static ssize_t __NC(pwrite)(int fd, const void *buf, size_t count, off_t offset)
{
	return MY_PWRITE(fd, buf, count, offset);
}
CANCELLABLE_SYSCALL(ssize_t, pwrite, (int fd, const void *buf, size_t count, off_t offset),
		    (fd, buf, count, offset))

#if __WORDSIZE == 32
static ssize_t __NC(pread64)(int fd, void *buf, size_t count, off64_t offset)
{
	return MY_PREAD64(fd, buf, count, offset);
}
CANCELLABLE_SYSCALL(ssize_t, pread64, (int fd, void *buf, size_t count, off64_t offset),
		    (fd, buf, count, offset))

static ssize_t __NC(pwrite64)(int fd, const void *buf, size_t count, off64_t offset)
{
	return MY_PWRITE64(fd, buf, count, offset);
}
CANCELLABLE_SYSCALL(ssize_t, pwrite64, (int fd, const void *buf, size_t count, off64_t offset),
		    (fd, buf, count, offset))
#else
# ifdef __UCLIBC_HAS_LINUXTHREADS__
weak_alias(pread,pread64)
weak_alias(pwrite,pwrite64)
lt_strong_alias(pread64)
lt_strong_alias(pwrite64)
# else
strong_alias_untyped(pread,pread64)
strong_alias_untyped(pwrite,pwrite64)
# endif
#endif
