#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include "dirstream.h"


/* opendir just makes an open() call - it return NULL if it fails
 * (open sets errno), otherwise it returns a DIR * pointer.
 */
DIR *opendir(const char *name)
{
	int fd;
	struct stat statbuf;
	char *buf;
	DIR *ptr;

	if (stat(name, &statbuf))
		return NULL;
	if (!S_ISDIR(statbuf.st_mode)) {
		__set_errno(ENOTDIR);
		return NULL;
	}
	if ((fd = open(name, O_RDONLY)) < 0)
		return NULL;
	/* According to POSIX, directory streams should be closed when
	 * exec. From "Anna Pluzhnikov" <besp@midway.uchicago.edu>.
	 */
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
		return NULL;
	if (!(ptr = malloc(sizeof(*ptr)))) {
		close(fd);
		__set_errno(ENOMEM);
		return NULL;
	}

	ptr->dd_fd = fd;
	ptr->dd_nextloc = ptr->dd_size = ptr->dd_nextoff = 0;

	ptr->dd_max = statbuf.st_blksize;
	if (ptr->dd_max < 512)
		ptr->dd_max = 512;

	if (!(buf = calloc(1, ptr->dd_max))) {
		close(fd);
		free(ptr);
		__set_errno(ENOMEM);
		return NULL;
	}
	ptr->dd_buf = buf;
#ifdef __UCLIBC_HAS_THREADS__
	__pthread_mutex_init(&(ptr->dd_lock), NULL);
#endif
	return ptr;
}
