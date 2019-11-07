/* Convert between the kernel's `struct stat' format, and libc's.
   Copyright (C) 1991,1995,1996,1997,2000,2002 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.

   Modified for uClibc by Erik Andersen <andersen@codepoet.org>
   */

#include <sys/stat.h>
#include <string.h>
#include "xstatconv.h"

void __xstat_conv(struct kernel_stat *kbuf, struct stat *buf)
{
	/* Convert to current kernel version of `struct stat'. */
	memset(buf, 0x00, sizeof(*buf));
	buf->st_dev = kbuf->st_dev;
	buf->st_ino = kbuf->st_ino;
	buf->st_mode = kbuf->st_mode;
	buf->st_nlink = kbuf->st_nlink;
	buf->st_uid = kbuf->st_uid;
	buf->st_gid = kbuf->st_gid;
	buf->st_rdev = kbuf->st_rdev;
	buf->st_size = kbuf->st_size;
	buf->st_blksize = kbuf->st_blksize;
	buf->st_blocks = kbuf->st_blocks;
	buf->st_atim.tv_sec = kbuf->st_atim.tv_sec;
	buf->st_atim.tv_nsec = kbuf->st_atim.tv_nsec;
	buf->st_mtim.tv_sec = kbuf->st_mtim.tv_sec;
	buf->st_mtim.tv_nsec = kbuf->st_mtim.tv_nsec;
	buf->st_ctim.tv_sec = kbuf->st_ctim.tv_sec;
	buf->st_ctim.tv_nsec = kbuf->st_ctim.tv_nsec;
}

void __xstat32_conv(struct kernel_stat64 *kbuf, struct stat *buf)
{
	/* Convert to current kernel version of `struct stat64'. */
	memset(buf, 0x00, sizeof(*buf));
	buf->st_dev = kbuf->st_dev;
	buf->st_ino = kbuf->st_ino;
	buf->st_mode = kbuf->st_mode;
	buf->st_nlink = kbuf->st_nlink;
	buf->st_uid = kbuf->st_uid;
	buf->st_gid = kbuf->st_gid;
	buf->st_rdev = kbuf->st_rdev;
	buf->st_size = kbuf->st_size;
	buf->st_blksize = kbuf->st_blksize;
	buf->st_blocks = kbuf->st_blocks;
	buf->st_atim.tv_sec = kbuf->st_atim.tv_sec;
	buf->st_atim.tv_nsec = kbuf->st_atim.tv_nsec;
	buf->st_mtim.tv_sec = kbuf->st_mtim.tv_sec;
	buf->st_mtim.tv_nsec = kbuf->st_mtim.tv_nsec;
	buf->st_ctim.tv_sec = kbuf->st_ctim.tv_sec;
	buf->st_ctim.tv_nsec = kbuf->st_ctim.tv_nsec;
}

void __xstat64_conv(struct kernel_stat64 *kbuf, struct stat64 *buf)
{
	/* Convert to current kernel version of `struct stat64'. */
	memset(buf, 0x00, sizeof(*buf));
	buf->st_dev = kbuf->st_dev;
	buf->st_ino = kbuf->st_ino;
# ifdef _HAVE_STAT64___ST_INO
	buf->__st_ino = kbuf->__st_ino;
# endif
	buf->st_mode = kbuf->st_mode;
	buf->st_nlink = kbuf->st_nlink;
	buf->st_uid = kbuf->st_uid;
	buf->st_gid = kbuf->st_gid;
	buf->st_rdev = kbuf->st_rdev;
	buf->st_size = kbuf->st_size;
	buf->st_blksize = kbuf->st_blksize;
	buf->st_blocks = kbuf->st_blocks;
	buf->st_atim.tv_sec = kbuf->st_atim.tv_sec;
	buf->st_atim.tv_nsec = kbuf->st_atim.tv_nsec;
	buf->st_mtim.tv_sec = kbuf->st_mtim.tv_sec;
	buf->st_mtim.tv_nsec = kbuf->st_mtim.tv_nsec;
	buf->st_ctim.tv_sec = kbuf->st_ctim.tv_sec;
	buf->st_ctim.tv_nsec = kbuf->st_ctim.tv_nsec;
}
