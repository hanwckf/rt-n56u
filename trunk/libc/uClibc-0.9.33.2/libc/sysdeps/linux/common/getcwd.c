/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* These functions find the absolute path to the current working directory.  */

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/syscall.h>



#ifdef __NR_getcwd

# define __NR___syscall_getcwd __NR_getcwd
static __always_inline
_syscall2(int, __syscall_getcwd, char *, buf, unsigned long, size)

#else

/* If the syscall is not present, we have to walk up the
 * directory tree till we hit the root.  Now we _could_
 * use /proc/self/cwd if /proc is mounted... That approach
 * is left an an exercise for the reader... */


/* Seems a few broken filesystems (like coda) don't like this */
/* #undef FAST_DIR_SEARCH_POSSIBLE on Linux */


/* Routine to find the step back down */
static char *search_dir(dev_t this_dev, ino_t this_ino, char *path_buf, int path_size)
{
	DIR *dp;
	struct dirent *d;
	char *ptr;
	int slen;
	struct stat st;

# ifdef FAST_DIR_SEARCH_POSSIBLE
	/* The test is for ELKS lib 0.0.9, this should be fixed in the real kernel */
	int slow_search = (sizeof(ino_t) != sizeof(d->d_ino));
# endif

	if (stat(path_buf, &st) < 0) {
		goto oops;
	}
# ifdef FAST_DIR_SEARCH_POSSIBLE
	if (this_dev != st.st_dev)
		slow_search = 1;
# endif

	slen = strlen(path_buf);
	ptr = path_buf + slen - 1;
	if (*ptr != '/') {
		if (slen + 2 > path_size) {
			goto oops;
		}
		strcpy(++ptr, "/");
		slen++;
	}
	slen++;

	dp = opendir(path_buf);
	if (!dp) {
	    goto oops;
	}

	while ((d = readdir(dp)) != 0) {
# ifdef FAST_DIR_SEARCH_POSSIBLE
		if (slow_search || this_ino == d->d_ino) {
# endif
			if (slen + strlen(d->d_name) > path_size) {
			    closedir(dp);
			    goto oops;
			}
			strcpy(ptr + 1, d->d_name);
			if (stat(path_buf, &st) < 0)
				continue;
			if (st.st_ino == this_ino && st.st_dev == this_dev) {
				closedir(dp);
				return path_buf;
			}
# ifdef FAST_DIR_SEARCH_POSSIBLE
		}
# endif
	}

	closedir(dp);
	return 0;

oops:
	__set_errno(ERANGE);
	return 0;
}

/* Routine to go up tree */
static char *recurser(char *path_buf, int path_size, dev_t root_dev, ino_t root_ino)
{
	struct stat st;
	dev_t this_dev;
	ino_t this_ino;

	if (stat(path_buf, &st) < 0) {
	    if (errno != EFAULT)
		goto oops;
	    return 0;
	}
	this_dev = st.st_dev;
	this_ino = st.st_ino;
	if (this_dev == root_dev && this_ino == root_ino) {
		if (path_size < 2) {
		    goto oops;
		}
		strcpy(path_buf, "/");
		return path_buf;
	}
	if (strlen(path_buf) + 4 > path_size) {
	    goto oops;
	}
	strcat(path_buf, "/..");
	if (recurser(path_buf, path_size, root_dev, root_ino) == 0)
		return 0;

	return search_dir(this_dev, this_ino, path_buf, path_size);
oops:
	__set_errno(ERANGE);
	return 0;
}

static __always_inline
int __syscall_getcwd(char * buf, unsigned long size)
{
    int len;
    char *cwd;
    struct stat st;
    int olderrno;

    olderrno = errno;
    len = -1;

    /* get stat for root to have a valid parameters for the terminating condition */
    if (stat("/", &st) < 0) {
	/* root dir not found! */
	return -1;
    }
    /* start with actual dir */
    if (buf) strncpy(buf, ".", size);

    cwd = recurser(buf, size, st.st_dev, st.st_ino);
    if (cwd) {
	len = strlen(buf) + 1;
	__set_errno(olderrno);
    }
    return len;
}

#endif /* __NR_getcwd */

char *getcwd(char *buf, size_t size)
{
    int ret;
    char *path;
    size_t alloc_size = size;

    if (size == 0) {
	if (buf != NULL) {
	    __set_errno(EINVAL);
	    return NULL;
	}
	alloc_size = MAX (PATH_MAX, getpagesize ());
    }
    path=buf;
    if (buf == NULL) {
	path = malloc(alloc_size);
	if (path == NULL)
	    return NULL;
    }
    ret = __syscall_getcwd(path, alloc_size);
    if (ret >= 0)
    {
	if (buf == NULL && size == 0)
	    buf = realloc(path, ret);
	if (buf == NULL)
	    buf = path;
	return buf;
    }
    if (buf == NULL)
	free (path);
    return NULL;
}
libc_hidden_def(getcwd)
