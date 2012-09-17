/* Copyright (C) 2009 Bernhard Reutner-Fischer <uclibc@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifndef _PATH_SHM
#define _PATH_SHM "/dev/shm/"
#endif

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

/* Get name of dummy shm operation handle.
 * Returns a malloc'ed buffer containing the OS specific path
 * to the shm filename or NULL upon failure.
 */
static __attribute_noinline__ char* get_shm_name(const char *name) __nonnull((1));
static char* get_shm_name(const char *name)
{
	char *path;
	int i;

	/* Skip leading slashes */
	while (*name == '/')
		++name;
#ifdef __USE_GNU
	i = asprintf(&path, _PATH_SHM "%s", name);
	if (i < 0)
		return NULL;
#else
	path = malloc(NAME_MAX);
	if (path == NULL)
		return NULL;
	i = snprintf(path, NAME_MAX, _PATH_SHM "%s", name);
	if (i < 0) {
		free(path);
		return NULL;
	} else if (i >= NAME_MAX) {
		free(path);
		__set_errno(ENAMETOOLONG);
		return NULL;
	}
#endif
	return path;
}

int shm_open(const char *name, int oflag, mode_t mode)
{
	int fd;
	char *shm_name = get_shm_name(name);

	/* Stripped multiple '/' from start; may have set errno properly */
	if (shm_name == NULL)
		return -1;
	/* The FD_CLOEXEC file descriptor flag associated with the new
	 * file descriptor is set.  */
#ifdef O_CLOEXEC
	 /* Just open it with CLOEXEC set, for brevity */
	fd = open(shm_name, oflag | O_CLOEXEC, mode);
#else
	fd = open(shm_name, oflag, mode);
	if (fd >= 0) {
		fcntl(fd, F_SETFD, FD_CLOEXEC);
		/* thus far, {G,S}ETFD only has this single flag,
		 * and setting it never fails.
		 *int fdflags = fcntl(fd, F_GETFD);
		 *if (fdflags >= 0)
		 *	fdflags = fcntl(fd, F_SETFD, fdflags | FD_CLOEXEC);
		 *if (fdflags < 0) {
		 *	close(fd);
		 *	fd = -1;
		 *}
		 */
	}
#endif
	free(shm_name); /* doesn't affect errno */
	return fd;
}

int shm_unlink(const char *name)
{
	char *shm_name = get_shm_name(name);
	int ret;

	/* Stripped multiple '/' from start; may have set errno properly */
	if (shm_name == NULL)
		return -1;
	ret = unlink(shm_name);
	free(shm_name); /* doesn't affect errno */
	return ret;
}
