/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	POSIX Standard: 5.1.2 Directory Operations	<dirent.h>
 */

#ifndef	_DIRSTREAM_H

#define	_DIRSTREAM_H	1

#include <features.h>
#include <sys/types.h>

#include <bits/uClibc_mutex.h>

/* For now, syscall readdir () only supports one entry at a time. It
 * will be changed in the future.
#define NUMENT		3
*/
#ifndef NUMENT
#define NUMENT		1
#endif

#define SINGLE_READDIR	11
#define MULTI_READDIR	12
#define NEW_READDIR	13

/* Directory stream type.  */
struct __dirstream {
  /* file descriptor */
  int dd_fd;

  /* offset of the next dir entry in buffer */
  size_t dd_nextloc;

  /* bytes of valid entries in buffer */
  size_t dd_size;

  /* -> directory buffer */
  void *dd_buf;

  /* offset of the next dir entry in directory. */
  off_t dd_nextoff;

  /* total size of buffer */
  size_t dd_max;

  /* lock */
  __UCLIBC_MUTEX(dd_lock);
};				/* stream data from opendir() */


extern ssize_t __getdents(int fd, char *buf, size_t count) attribute_hidden;
#ifdef __UCLIBC_HAS_LFS__
extern ssize_t __getdents64 (int fd, char *buf, size_t count) attribute_hidden;
#endif

#endif /* dirent.h  */
