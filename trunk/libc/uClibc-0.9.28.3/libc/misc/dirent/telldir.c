#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "dirstream.h"


long int telldir(DIR * dir)
{
	if (!dir) {
		__set_errno(EBADF);
		return -1;
	}

	/* The next entry. */
	return dir->dd_nextoff;
}
