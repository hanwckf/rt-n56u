/* Definitions of functions to access `dev_t' values.
   Copyright (C) 2003-2015 Free Software Foundation, Inc.
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

#include <endian.h>
#include <sys/sysmacros.h>

unsigned int
gnu_dev_major (unsigned long long int dev)
{
  return ((dev >> 8) & 0xfff) | ((unsigned int) (dev >> 32) & ~0xfff);
}
libc_hidden_def(gnu_dev_major)

unsigned int
gnu_dev_minor (unsigned long long int dev)
{
  return (dev & 0xff) | ((unsigned int) (dev >> 12) & ~0xff);
}
libc_hidden_def(gnu_dev_minor)

unsigned long long int
gnu_dev_makedev (unsigned int major, unsigned int minor)
{
  return ((minor & 0xff) | ((major & 0xfff) << 8)
	  | (((unsigned long long int) (minor & ~0xff)) << 12)
	  | (((unsigned long long int) (major & ~0xfff)) << 32));
}
