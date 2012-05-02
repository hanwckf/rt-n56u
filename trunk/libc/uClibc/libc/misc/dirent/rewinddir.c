#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "dirstream.h"

/* rewinddir() just does an lseek(fd,0,0) - see close for comments */
void rewinddir(DIR * dir)
{
	if (!dir) {
		__set_errno(EBADF);
		return;
	}
	__UCLIBC_MUTEX_LOCK(dir->dd_lock);
	lseek(dir->dd_fd, 0, SEEK_SET);
	dir->dd_nextoff = dir->dd_nextloc = dir->dd_size = 0;
	__UCLIBC_MUTEX_UNLOCK(dir->dd_lock);
}
