/* vi: set sw=4 ts=4: */
/*
 * llseek/lseek64 syscall for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

/* Newer kernel ports have llseek() instead of _llseek() */
#if !defined __NR__llseek && defined __NR_llseek
# define __NR__llseek __NR_llseek
#endif

#if defined __NR__llseek && defined __UCLIBC_HAS_LFS__

loff_t lseek64(int fd, loff_t offset, int whence)
{
	loff_t result;
	return (loff_t)(INLINE_SYSCALL(_llseek, 5, fd, (off_t) (offset >> 32),
				(off_t) (offset & 0xffffffff), &result, whence) ?: result);
}

#else

loff_t lseek64(int fd, loff_t offset, int whence)
{
	return (loff_t)(lseek(fd, (off_t) (offset), whence));
}

#endif

#ifndef __LINUXTHREADS_OLD__
libc_hidden_def(lseek64)
#else
libc_hidden_weak(lseek64)
strong_alias(lseek64,__libc_lseek64)
#endif
