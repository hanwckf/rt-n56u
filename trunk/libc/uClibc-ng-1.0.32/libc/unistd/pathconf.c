/* Copyright (C) 1991,1995,1996,1998,2000,2001,2003
   Free Software Foundation, Inc.
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

/* pathconf -- adjusted for busybox */

/* It would be great it this could be implemented using fpathconf,
 * but that doesn't work out very well (think FIFOs and sockets) */

#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statfs.h>

extern __typeof(statfs) __libc_statfs;


/* The Linux kernel headers mention this as a kind of generic value. */
#ifndef LINK_MAX
# define LINK_MAX 127
#endif

/* Get file-specific information about PATH.  */
long int
pathconf (const char *path, int name)
{
  if (path[0] == '\0')
    {
      __set_errno (ENOENT);
      return -1;
    }

  switch (name)
    {
    default:
      __set_errno (EINVAL);
      return -1;

    case _PC_LINK_MAX:
#ifdef	LINK_MAX
      return LINK_MAX;
#else
      return -1;
#endif

    case _PC_MAX_CANON:
#ifdef	MAX_CANON
      return MAX_CANON;
#else
      return -1;
#endif

    case _PC_MAX_INPUT:
#ifdef	MAX_INPUT
      return MAX_INPUT;
#else
      return -1;
#endif

    case _PC_NAME_MAX:
#ifdef	NAME_MAX
      {
	struct statfs buf;
	int save_errno = errno;

	if (__libc_statfs (path, &buf) < 0)
	  {
	    if (errno == ENOSYS)
	      {
		errno = save_errno;
		return NAME_MAX;
	      }
	    return -1;
	  }
	else
	  {
#ifdef _STATFS_F_NAMELEN
	    return buf.f_namelen;
#else
# ifdef _STATFS_F_NAME_MAX
	    return buf.f_name_max;
# else
	    return NAME_MAX;
# endif
#endif
	  }
      }
#else
      return -1;
#endif

    case _PC_PATH_MAX:
#ifdef	PATH_MAX
      return PATH_MAX;
#else
      return -1;
#endif

    case _PC_PIPE_BUF:
#ifdef	PIPE_BUF
      return PIPE_BUF;
#else
      return -1;
#endif

    case _PC_CHOWN_RESTRICTED:
#ifdef	_POSIX_CHOWN_RESTRICTED
      return _POSIX_CHOWN_RESTRICTED;
#else
      return -1;
#endif

    case _PC_NO_TRUNC:
#ifdef	_POSIX_NO_TRUNC
      return _POSIX_NO_TRUNC;
#else
      return -1;
#endif

    case _PC_VDISABLE:
#ifdef	_POSIX_VDISABLE
      return _POSIX_VDISABLE;
#else
      return -1;
#endif

    case _PC_SYNC_IO:
#ifdef	_POSIX_SYNC_IO
      return _POSIX_SYNC_IO;
#else
      return -1;
#endif

    case _PC_ASYNC_IO:
#if defined _POSIX_ASYNC_IO
      {
	/* AIO is only allowed on regular files and block devices.  */
	struct stat st;

	if (stat (path, &st) < 0
	    || (! S_ISREG (st.st_mode) && ! S_ISBLK (st.st_mode)))
	  return -1;
	else
	  return 1;
      }
#else
      return -1;
#endif

    case _PC_PRIO_IO:
#ifdef	_POSIX_PRIO_IO
      return _POSIX_PRIO_IO;
#else
      return -1;
#endif

    case _PC_SOCK_MAXBUF:
#ifdef	SOCK_MAXBUF
      return SOCK_MAXBUF;
#else
      return -1;
#endif

    case _PC_FILESIZEBITS:
#ifdef FILESIZEBITS
      return FILESIZEBITS;
#else
      /* We let platforms with larger file sizes overwrite this value.  */
      return 32;
#endif

        /* Be lazy -- skip these */
    case _PC_REC_INCR_XFER_SIZE:
    case _PC_REC_MAX_XFER_SIZE:
    case _PC_REC_MIN_XFER_SIZE:
    case _PC_REC_XFER_ALIGN:
    case _PC_ALLOC_SIZE_MIN:
    case _PC_SYMLINK_MAX:
      return -1;
    }
}
