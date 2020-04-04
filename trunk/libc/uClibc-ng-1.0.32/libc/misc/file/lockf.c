/* Copyright (C) 1994, 1996, 1997, 1998, 2000 Free Software Foundation, Inc.
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

#include <features.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* lockf is a simplified interface to fcntl's locking facilities.  */

int lockf (int fd, int cmd, off_t len)
{
    struct flock fl;

    memset ((char *) &fl, '\0', sizeof (fl));

    /* lockf is always relative to the current file position.  */
    fl.l_whence = SEEK_CUR;
    fl.l_start = 0;
    fl.l_len = len;

    switch (cmd)
    {
	case F_TEST:
	    /* Test the lock: return 0 if FD is unlocked or locked by this process;
	       return -1, set errno to EACCES, if another process holds the lock.  */
	    fl.l_type = F_RDLCK;
	    if (fcntl (fd, F_GETLK, &fl) < 0)
		return -1;
	    if (fl.l_type == F_UNLCK || fl.l_pid == getpid ())
		return 0;
	    __set_errno(EACCES);
	    return -1;

	case F_ULOCK:
	    fl.l_type = F_UNLCK;
	    cmd = F_SETLK;
	    break;
	case F_LOCK:
	    fl.l_type = F_WRLCK;
	    cmd = F_SETLKW;
	    break;
	case F_TLOCK:
	    fl.l_type = F_WRLCK;
	    cmd = F_SETLK;
	    break;

	default:
	    __set_errno(EINVAL);
	    return -1;
    }

    return fcntl(fd, cmd, &fl);
}
libc_hidden_def(lockf)
