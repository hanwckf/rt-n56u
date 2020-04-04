/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "dirstream.h"
#include <not-cancel.h>


int closedir(DIR * dir)
{
	int fd;

	if (!dir) {
		__set_errno(EBADF);
		return -1;
	}

	/* We need to check dd_fd. */
	if (dir->dd_fd == -1) {
		__set_errno(EBADF);
		return -1;
	}
	__UCLIBC_MUTEX_LOCK(dir->dd_lock);
	fd = dir->dd_fd;
	dir->dd_fd = -1;
	__UCLIBC_MUTEX_UNLOCK(dir->dd_lock);
	free(dir->dd_buf);
	free(dir);
	return close_not_cancel(fd);
}
libc_hidden_def(closedir)
