/* Provide kernel hint to read ahead.
   Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sys/syscall.h>

#if defined __NR_readahead && defined __USE_GNU

# include <fcntl.h>
# include <bits/wordsize.h>

# if __WORDSIZE == 64

_syscall3(ssize_t, readahead, int, fd, off_t, offset, size_t, count)

# else

ssize_t readahead(int fd, off64_t offset, size_t count)
{
	return INLINE_SYSCALL(readahead,
#  if defined(__UCLIBC_SYSCALL_ALIGN_64BIT__)
		5, fd, 0,
#  else
		4, fd,
#  endif
		OFF64_HI_LO(offset), count);
}

# endif

#endif
