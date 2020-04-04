/* Copyright (C) 2009-2016 Free Software Foundation, Inc.
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

#include <sys/uio.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_preadv
ssize_t
preadv (int fd, const struct iovec *vector, int count, __off64_t offset)
{
  unsigned long pos_l, pos_h;

  pos_h = (unsigned long)((long long)offset >> 32);
  pos_l = (unsigned long)((long long)offset);	

  return INLINE_SYSCALL (preadv, 5, fd, vector, count, pos_l, pos_h);
}
#endif
