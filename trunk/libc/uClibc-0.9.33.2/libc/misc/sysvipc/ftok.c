/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sys/ipc.h>
#include <sys/stat.h>
#ifdef __UCLIBC_HAS_LFS__
# include <_lfs_64.h>
#else
# define stat64 stat
#endif

key_t ftok (const char *pathname, int proj_id)
{
  struct stat64 st;
  key_t key;

  if (stat64(pathname, &st) < 0)
    return (key_t) -1;

  key = ((st.st_ino & 0xffff) | ((st.st_dev & 0xff) << 16)
	 | ((proj_id & 0xff) << 24));

  return key;
}
