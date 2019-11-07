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
not, see <http://www.gnu.org/licenses/>.  */

/*
 *	POSIX Standard: 5.1.2 Directory Operations	<dirent.h>
 */

#ifndef	_DIRSTREAM_H

#define	_DIRSTREAM_H	1

#include <features.h>
#include <sys/types.h>

#include <bits/uClibc_mutex.h>

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

#endif /* dirent.h  */
