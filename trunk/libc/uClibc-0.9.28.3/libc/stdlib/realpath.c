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
#include <strings.h>
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

#ifdef __STDC__
char *realpath(const char *path, char resolved_path[])
#else
char *realpath(path, resolved_path)
const char *path;
char resolved_path[];
#endif
{
	char copy_path[PATH_MAX];
	char link_path[PATH_MAX];
	char got_path[PATH_MAX];
	char *new_path = got_path;
	char *max_path;
	int readlinks = 0;
	int n;
	int is_buffer_malloc = 0;

	if (path == NULL) {
		__set_errno(EINVAL);
		return NULL;
	}
	if (*path == '\0') {
		__set_errno(ENOENT);
		return NULL;
	}
	/* Make a copy of the source path since we may need to modify it. */
	if (strlen(path) >= PATH_MAX - 2) {
		__set_errno(ENAMETOOLONG);
		return NULL;
	}
	strcpy(copy_path, path);
	path = copy_path;
	max_path = copy_path + PATH_MAX - 2;
	/* If it's a relative pathname use getcwd for starters. */
	if (*path != '/') {
		/* Ohoo... */
		getcwd(new_path, PATH_MAX - 1);
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
			if (path > max_path) {
				__set_errno(ENAMETOOLONG);
				return NULL;
			}
			*new_path++ = *path++;
		}
#ifdef S_IFLNK
		/* Protect against infinite loops. */
		if (readlinks++ > MAX_READLINKS) {
			__set_errno(ELOOP);
			return NULL;
		}
		/* See if latest pathname component is a symlink. */
		*new_path = '\0';
		n = readlink(got_path, link_path, PATH_MAX - 1);
		/* Add this to prevent from segmentation fault when resolved_path is NULL */
		if(!resolved_path) {
			resolved_path = malloc(PATH_MAX);
			if(!resolved_path)
				return NULL;
			is_buffer_malloc = 1;
		}
		if (n < 0) {
			/* EINVAL means the file exists but isn't a symlink. */
			if (errno != EINVAL) {
				/* Make sure it's null terminated. */
				*new_path = '\0';
				strcpy(resolved_path, got_path);
				if(is_buffer_malloc)
					free(resolved_path);
				return NULL;
			}
		} else {
			/* Note: readlink doesn't add the null byte. */
			link_path[n] = '\0';
			if (*link_path == '/')
				/* Start over for an absolute symlink. */
				new_path = got_path;
			else
				/* Otherwise back up over this component. */
				while (*(--new_path) != '/');
			/* Safe sex check. */
			if (strlen(path) + n >= PATH_MAX - 2) {
				__set_errno(ENAMETOOLONG);
				if(is_buffer_malloc)
					free(resolved_path);
				return NULL;
			}
			/* Insert symlink contents into path. */
			strcat(link_path, path);
			strcpy(copy_path, link_path);
			path = copy_path;
		}
#endif							/* S_IFLNK */
		*new_path++ = '/';
	}
	/* Delete trailing slash but don't whomp a lone slash. */
	if (new_path != got_path + 1 && new_path[-1] == '/')
		new_path--;
	/* Make sure it's null terminated. */
	*new_path = '\0';
	strcpy(resolved_path, got_path);
	return resolved_path;
}
