/*
 * truncate() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#if defined(__NR_truncate64) && !defined(__NR_truncate)
# include <endian.h>
# include <stdint.h>

int truncate(const char *path, __off_t length)
{
# if __WORDSIZE == 32
#  if defined(__UCLIBC_SYSCALL_ALIGN_64BIT__)
	return INLINE_SYSCALL(truncate64, 4, path, 0, OFF_HI_LO(length));
#  else
	return INLINE_SYSCALL(truncate64, 3, path, OFF_HI_LO(length));
#  endif
# else
	return truncate64(path, length);
# endif
}
libc_hidden_def(truncate);

#else
_syscall2(int, truncate, const char *, path, __off_t, length)
libc_hidden_def(truncate)
#endif
