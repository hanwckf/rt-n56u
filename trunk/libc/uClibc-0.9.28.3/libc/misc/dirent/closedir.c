#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "dirstream.h"

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
	return close(fd);
}
