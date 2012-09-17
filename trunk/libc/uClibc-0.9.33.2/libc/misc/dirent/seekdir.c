/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "dirstream.h"


void seekdir(DIR * dir, long int offset)
{
	if (!dir) {
		__set_errno(EBADF);
		return;
	}
	__UCLIBC_MUTEX_LOCK(dir->dd_lock);
	dir->dd_nextoff = lseek(dir->dd_fd, offset, SEEK_SET);
	dir->dd_size = dir->dd_nextloc = 0;
	__UCLIBC_MUTEX_UNLOCK(dir->dd_lock);
}
