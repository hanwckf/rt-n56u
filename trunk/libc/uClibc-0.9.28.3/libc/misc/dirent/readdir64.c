#include <features.h>
#ifdef __UCLIBC_HAS_LFS__
#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS != 64 
#undef _FILE_OFFSET_BITS
#define	_FILE_OFFSET_BITS   64
#endif
#ifndef __USE_LARGEFILE64
# define __USE_LARGEFILE64	1
#endif
/* We absolutely do _NOT_ want interfaces silently
 * renamed under us or very bad things will happen... */
#ifdef __USE_FILE_OFFSET64
# undef __USE_FILE_OFFSET64
#endif
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "dirstream.h"

struct dirent64 *readdir64(DIR * dir)
{
	ssize_t bytes;
	struct dirent64 *de;

	if (!dir) {
		__set_errno(EBADF);
		return NULL;
	}

	__UCLIBC_MUTEX_LOCK(dir->dd_lock);

	do {
	    if (dir->dd_size <= dir->dd_nextloc) {
		/* read dir->dd_max bytes of directory entries. */
		bytes = __getdents64(dir->dd_fd, dir->dd_buf, dir->dd_max);
		if (bytes <= 0) {
		    de = NULL;
		    goto all_done;
		}
		dir->dd_size = bytes;
		dir->dd_nextloc = 0;
	    }

	    de = (struct dirent64 *) (((char *) dir->dd_buf) + dir->dd_nextloc);

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

#endif /* __UCLIBC_HAS_LFS__ */
