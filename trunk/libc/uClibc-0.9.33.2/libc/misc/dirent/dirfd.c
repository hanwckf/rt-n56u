/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <dirent.h>
#include <errno.h>
#include "dirstream.h"


int dirfd(DIR * dir)
{
	if (!dir || dir->dd_fd == -1) {
		__set_errno(EBADF);
		return -1;
	}

	return dir->dd_fd;
}
libc_hidden_def(dirfd)
