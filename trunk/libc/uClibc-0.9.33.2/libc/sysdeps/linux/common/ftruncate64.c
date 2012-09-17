/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * ftruncate64 syscall.  Copes with 64 bit and 32 bit machines
 * and on 32 bit machines this sends things into the kernel as
 * two 32-bit arguments (high and low 32 bits of length) that
 * are ordered based on endianess.  It turns out endian.h has
 * just the macro we need to order things, __LONG_LONG_PAIR.
 */

#include <features.h>

#ifdef __UCLIBC_HAS_LFS__

# include <unistd.h>
# include <errno.h>
# include <endian.h>
# include <stdint.h>
# include <sys/types.h>
# include <sys/syscall.h>


# ifdef __NR_ftruncate64

#  if __WORDSIZE == 64

/* For a 64 bit machine, life is simple... */
_syscall2(int, ftruncate64, int, fd, __off64_t, length)

#  elif __WORDSIZE == 32

/* The exported ftruncate64 function.  */
int ftruncate64 (int fd, __off64_t length)
{
    uint32_t low = length & 0xffffffff;
    uint32_t high = length >> 32;
#   if defined(__UCLIBC_TRUNCATE64_HAS_4_ARGS__)
    return INLINE_SYSCALL(ftruncate64,
	    4, fd, 0, __LONG_LONG_PAIR (high, low));
#   else
    return INLINE_SYSCALL(ftruncate64, 3, fd,
	    __LONG_LONG_PAIR (high, low));
#   endif
}

#  else /* __WORDSIZE */
#   error Your machine is not 64 bit or 32 bit, I am dazed and confused.
#  endif /* __WORDSIZE */

# else  /* __NR_ftruncate64 */


int ftruncate64 (int fd, __off64_t length)
{
	__off_t x = (__off_t) length;

	if (x == length) {
		return ftruncate(fd, x);
	}

	__set_errno((x < 0) ? EINVAL : EFBIG);

	return -1;
}

# endif /* __NR_ftruncate64 */
libc_hidden_def(ftruncate64)

#endif /* __UCLIBC_HAS_LFS__ */
