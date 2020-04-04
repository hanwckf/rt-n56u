/*
 * llseek/lseek64 syscall for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <bits/wordsize.h>

/* Newer kernel ports have llseek() instead of _llseek() */
#if !defined __NR__llseek && defined __NR_llseek
# define __NR__llseek __NR_llseek
#endif

#if defined __NR__llseek && __WORDSIZE == 32
# include <unistd.h>
# include <endian.h>
# include <cancel.h>
off64_t __NC(lseek64)(int fd, off64_t offset, int whence)
{
	off64_t result;
	/* do we not need to handle the offset with __LONG_LONG_PAIR depending on endianness? */
	return (off64_t)INLINE_SYSCALL(_llseek, 5, fd, (off_t) OFF64_HI(offset),
				       (off_t) OFF64_LO(offset), &result, whence) ?: result;
}
CANCELLABLE_SYSCALL(off64_t, lseek64, (int fd, off64_t offset, int whence), (fd, offset, whence))
lt_libc_hidden(lseek64)
#endif
