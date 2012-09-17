/* Copyright (C) 1991, 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <_lfs_64.h>

#ifdef __UCLIBC_HAS_LFS__
#include <fcntl.h>
#include <sys/types.h>

/* Create FILE with protections MODE.  */
int creat64(const char *file, mode_t mode)
{
	return open64(file, O_WRONLY|O_CREAT|O_TRUNC, mode);
}
#endif /* __UCLIBC_HAS_LFS__ */
