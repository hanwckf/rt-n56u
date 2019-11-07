/* Return information about the filesystem on which FD resides.
   Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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

#include <_lfs_64.h>

#include <errno.h>
#include <string.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/syscall.h>
#include <stddef.h>

#if !defined __NR_fstatfs64
extern __typeof(fstatfs) __libc_fstatfs;

/* Return information about the filesystem on which FD resides.  */
int fstatfs64 (int fd, struct statfs64 *buf)
{
    struct statfs buf32;

    if (__libc_fstatfs (fd, (struct statfs *)buf) < 0)
	return -1;

    buf32 = *(struct statfs *)buf;

    buf->f_type = buf32.f_type;
    buf->f_bsize = buf32.f_bsize;
    buf->f_blocks = buf32.f_blocks;
    buf->f_bfree = buf32.f_bfree;
    buf->f_bavail = buf32.f_bavail;
    buf->f_files = buf32.f_files;
    buf->f_ffree = buf32.f_ffree;
    buf->f_fsid = buf32.f_fsid;
# ifdef _STATFS_F_FRSIZE
    buf->f_frsize = buf32.f_frsize;
# endif
    buf->f_namelen = buf32.f_namelen;
    memcpy (buf->f_spare, buf32.f_spare, sizeof (buf32.f_spare));

    return 0;
}
#else
int fstatfs64(int fd, struct statfs64 *buf)
{
	/* Signature has 2 arguments but syscalls wants 3 */
	return INLINE_SYSCALL(fstatfs64, 3, fd, sizeof(*buf), buf);
}
#endif
libc_hidden_def(fstatfs64)
