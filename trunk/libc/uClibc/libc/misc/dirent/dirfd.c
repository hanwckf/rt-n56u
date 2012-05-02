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
