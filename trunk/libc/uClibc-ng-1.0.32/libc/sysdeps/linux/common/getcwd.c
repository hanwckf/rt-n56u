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


# define __NR___syscall_getcwd __NR_getcwd
static __always_inline
_syscall2(int, __syscall_getcwd, char *, buf, unsigned long, size)

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
    if (ret > 0 && path[0] == '/')
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
