/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * ftruncate64 syscall.  Copes with 64 bit and 32 bit machines
 * and on 32 bit machines this sends things into the kernel as
 * two 32-bit arguments (high and low 32 bits of length) that
 * are ordered based on endianess.  It turns out endian.h has
 * just the macro we need to order things, OFF64_HI_LO.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_ftruncate64
# include <bits/wordsize.h>

# if __WORDSIZE == 64

/* For a 64 bit machine, life is simple... */
_syscall2(int, ftruncate64, int, fd, __off64_t, length)

# elif __WORDSIZE == 32
#  include <endian.h>
#  include <stdint.h>

/* The exported ftruncate64 function.  */
int ftruncate64 (int fd, __off64_t length)
{
#  if defined(__UCLIBC_SYSCALL_ALIGN_64BIT__)
    return INLINE_SYSCALL(ftruncate64, 4, fd, 0, OFF64_HI_LO(length));
#  else
    return INLINE_SYSCALL(ftruncate64, 3, fd, OFF64_HI_LO(length));
#  endif
}

# else /* __WORDSIZE */
#  error Your machine is not 64 bit or 32 bit, I am dazed and confused.
# endif /* __WORDSIZE */

#else  /* __NR_ftruncate64 */
#  include <errno.h>

int ftruncate64 (int fd, __off64_t length)
{
	__off_t x = (__off_t) length;

	if (x == length) {
		return ftruncate(fd, x);
	}

	__set_errno((x < 0) ? EINVAL : EFBIG);

	return -1;
}

#endif /* __NR_ftruncate64 */
libc_hidden_def(ftruncate64)
