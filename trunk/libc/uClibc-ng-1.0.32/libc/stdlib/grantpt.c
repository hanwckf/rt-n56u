/* Copyright (C) 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
   see <http://www.gnu.org/licenses/>.  */

#include <limits.h>
#include <stdlib.h>

/* If __ASSUME_DEVPTS__ is defined, grantpt() reduces to a stub since we
   assume that the devfs/devpts filesystem automatically manages the
   permissions. */
#if !defined __ASSUME_DEVPTS__
#include <sys/statfs.h>

/* Constant that identifies the `devpts' filesystem.  */
#define DEVPTS_SUPER_MAGIC	0x1cd1
/* Constant that identifies the `devfs' filesystem.  */
#define DEVFS_SUPER_MAGIC	0x1373

/* Prototype for function that changes ownership and access permission
   for slave pseudo terminals that do not live on a `devpts'
   filesystem.  */
static int __unix_grantpt (int fd);

/* Prototype for private function that gets the name of the slave
   pseudo terminal in a safe way.  */
static int pts_name (int fd, char **pts, size_t buf_len);
extern __typeof(statfs) __libc_statfs;

/* Change the ownership and access permission of the slave pseudo
   terminal associated with the master pseudo terminal specified
   by FD.  */
int grantpt (int fd)
{
  struct statfs fsbuf;
  char _buf[PATH_MAX];
  char *buf = _buf;

  if (pts_name (fd, &buf, sizeof (_buf)))
    return -1;

  if (__libc_statfs (buf, &fsbuf) < 0)
    return -1;

  /* If the slave pseudo terminal lives on a `devpts' filesystem, the
     ownership and access permission are already set.  */
  if (fsbuf.f_type != DEVPTS_SUPER_MAGIC && fsbuf.f_type != DEVFS_SUPER_MAGIC)
    return __unix_grantpt (fd);

  return 0;
}

# define grantpt __unix_grantpt
# include "unix_grantpt.c"

#else

int grantpt (attribute_unused int fd)
{
  return 0;
}

#endif
