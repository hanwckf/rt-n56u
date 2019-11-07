/*
 * realpath.c -- canonicalize pathname by removing symlinks
 * Copyright (C) 1993 Rick Sladkey <jrs@world.std.com>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>				/* for PATH_MAX */
#include <sys/param.h>			/* for MAXPATHLEN */
#include <errno.h>
#include <stdlib.h>

#include <sys/stat.h>			/* for S_IFLNK */


#ifndef PATH_MAX
#ifdef _POSIX_VERSION
#define PATH_MAX _POSIX_PATH_MAX
#else
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif
#endif

#define MAX_READLINKS 32

char *realpath(const char *path, char got_path[])
{
	char copy_path[PATH_MAX];
	char *max_path, *new_path, *allocated_path;
	size_t path_len;
	int readlinks = 0;
#ifdef S_IFLNK
	int link_len;
#endif

	if (path == NULL) {
		__set_errno(EINVAL);
		return NULL;
	}
	if (*path == '\0') {
		__set_errno(ENOENT);
		return NULL;
	}
	/* Make a copy of the source path since we may need to modify it. */
	path_len = strlen(path);
	if (path_len >= PATH_MAX - 2) {
		__set_errno(ENAMETOOLONG);
		return NULL;
	}
	/* Copy so that path is at the end of copy_path[] */
	strcpy(copy_path + (PATH_MAX-1) - path_len, path);
	path = copy_path + (PATH_MAX-1) - path_len;
	allocated_path = got_path ? NULL : (got_path = malloc(PATH_MAX));
	max_path = got_path + PATH_MAX - 2; /* points to last non-NUL char */
	new_path = got_path;
	if (*path != '/') {
		/* If it's a relative pathname use getcwd for starters. */
		if (!getcwd(new_path, PATH_MAX - 1))
			goto err;
		new_path += strlen(new_path);
		if (new_path[-1] != '/')
			*new_path++ = '/';
	} else {
		*new_path++ = '/';
		path++;
	}
	/* Expand each slash-separated pathname component. */
	while (*path != '\0') {
		/* Ignore stray "/". */
		if (*path == '/') {
			path++;
			continue;
		}
		if (*path == '.') {
			/* Ignore ".". */
			if (path[1] == '\0' || path[1] == '/') {
				path++;
				continue;
			}
			if (path[1] == '.') {
				if (path[2] == '\0' || path[2] == '/') {
					path += 2;
					/* Ignore ".." at root. */
					if (new_path == got_path + 1)
						continue;
					/* Handle ".." by backing up. */
					while ((--new_path)[-1] != '/');
					continue;
				}
			}
		}
		/* Safely copy the next pathname component. */
		while (*path != '\0' && *path != '/') {
			if (new_path > max_path) {
				__set_errno(ENAMETOOLONG);
 err:
				free(allocated_path);
				return NULL;
			}
			*new_path++ = *path++;
		}
#ifdef S_IFLNK
		/* Protect against infinite loops. */
		if (readlinks++ > MAX_READLINKS) {
			__set_errno(ELOOP);
			goto err;
		}
		path_len = strlen(path);
		/* See if last (so far) pathname component is a symlink. */
		*new_path = '\0';
		{
			int sv_errno = errno;
			link_len = readlink(got_path, copy_path, PATH_MAX - 1);
			if (link_len < 0) {
				/* EINVAL means the file exists but isn't a symlink. */
				if (errno != EINVAL) {
					goto err;
				}
			} else {
				/* Safe sex check. */
				if (path_len + link_len >= PATH_MAX - 2) {
					__set_errno(ENAMETOOLONG);
					goto err;
				}
				/* Note: readlink doesn't add the null byte. */
				/* copy_path[link_len] = '\0'; - we don't need it too */
				if (*copy_path == '/')
					/* Start over for an absolute symlink. */
					new_path = got_path;
				else
					/* Otherwise back up over this component. */
					while (*(--new_path) != '/');
				/* Prepend symlink contents to path. */
				memmove(copy_path + (PATH_MAX-1) - link_len - path_len, copy_path, link_len);
				path = copy_path + (PATH_MAX-1) - link_len - path_len;
			}
			__set_errno(sv_errno);
		}
#endif							/* S_IFLNK */
		*new_path++ = '/';
	}
	/* Delete trailing slash but don't whomp a lone slash. */
	if (new_path != got_path + 1 && new_path[-1] == '/')
		new_path--;
	/* Make sure it's null terminated. */
	*new_path = '\0';
	return got_path;
}
libc_hidden_def(realpath)
