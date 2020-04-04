/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <dirent.h>
#include <errno.h>
#define __need_NULL
#include <stddef.h>
#include "dirstream.h"

#ifndef __READDIR
# define __READDIR readdir
# define __DIRENT_TYPE struct dirent
# define __GETDENTS __getdents
#endif

__DIRENT_TYPE *__READDIR(DIR * dir)
{
	ssize_t bytes;
	__DIRENT_TYPE *de;

	if (!dir) {
		__set_errno(EBADF);
		return NULL;
	}

	__UCLIBC_MUTEX_LOCK(dir->dd_lock);

	do {
	    if (dir->dd_size <= dir->dd_nextloc) {
		/* read dir->dd_max bytes of directory entries. */
		bytes = __GETDENTS(dir->dd_fd, dir->dd_buf, dir->dd_max);
		if (bytes <= 0) {
		    de = NULL;
		    goto all_done;
		}
		dir->dd_size = bytes;
		dir->dd_nextloc = 0;
	    }

	    de = (__DIRENT_TYPE *) (((char *) dir->dd_buf) + dir->dd_nextloc);

	    /* Am I right? H.J. */
	    dir->dd_nextloc += de->d_reclen;

	    /* We have to save the next offset here. */
	    dir->dd_nextoff = de->d_off;

	    /* Skip deleted files.  */
	} while (de->d_ino == 0);

all_done:
	__UCLIBC_MUTEX_UNLOCK(dir->dd_lock);
	return de;
}
libc_hidden_def(__READDIR)
#if __WORDSIZE == 64
strong_alias_untyped(readdir,readdir64)
libc_hidden_def(readdir64)
#endif
