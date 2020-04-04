/*
 * chroot_realpath.c -- resolve pathname as if inside chroot
 * Based on realpath.c Copyright (C) 1993 Rick Sladkey <jrs@world.std.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING.LIB.  If not,
 * see <http://www.gnu.org/licenses/>.
 *
 * 2005/09/12: Dan Howell (modified from realpath.c to emulate chroot)
 */

#include "porting.h"

#define MAX_READLINKS 32

char *chroot_realpath(const char *root, const char *path,
		      char resolved_path[]);

char *chroot_realpath(const char *root, const char *path,
		      char resolved_path[])
{
	char copy_path[PATH_MAX];
	char link_path[PATH_MAX];
	char got_path[PATH_MAX];
	char *got_path_root = got_path;
	char *new_path = got_path;
	char *max_path;
	int readlinks = 0;
	int n;
	int chroot_len;

	/* Trivial case. */
	if (root == NULL || *root == '\0' ||
	    (*root == '/' && root[1] == '\0')) {
		strcpy(resolved_path, path);
		return resolved_path;
	}

	chroot_len = strlen(root);

	if (chroot_len + strlen(path) >= PATH_MAX - 3) {
		errno = ENAMETOOLONG;
		return NULL;
	}

	/* Make a copy of the source path since we may need to modify it. */
	strcpy(copy_path, path);
	path = copy_path;
	max_path = copy_path + PATH_MAX - chroot_len - 3;

	/* Start with the chroot path. */
	strcpy(new_path, root);
	new_path += chroot_len;
	while (*new_path == '/' && new_path > got_path)
		new_path--;
	got_path_root = new_path;
	*new_path++ = '/';

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
					if (new_path == got_path_root + 1)
						continue;
					/* Handle ".." by backing up. */
					while ((--new_path)[-1] != '/') ;
					continue;
				}
			}
		}
		/* Safely copy the next pathname component. */
		while (*path != '\0' && *path != '/') {
			if (path > max_path) {
				errno = ENAMETOOLONG;
				return NULL;
			}
			*new_path++ = *path++;
		}
		if (*path == '\0')
			/* Don't follow symlink for last pathname component. */
			break;
#ifdef S_IFLNK
		/* Protect against infinite loops. */
		if (readlinks++ > MAX_READLINKS) {
			errno = ELOOP;
			return NULL;
		}
		/* See if latest pathname component is a symlink. */
		*new_path = '\0';
		n = readlink(got_path, link_path, PATH_MAX - 1);
		if (n < 0) {
			/* EINVAL means the file exists but isn't a symlink. */
			if (errno != EINVAL) {
				/* Make sure it's null terminated. */
				*new_path = '\0';
				strcpy(resolved_path, got_path);
				return NULL;
			}
		} else {
			/* Note: readlink doesn't add the null byte. */
			link_path[n] = '\0';
			if (*link_path == '/')
				/* Start over for an absolute symlink. */
				new_path = got_path_root;
			else
				/* Otherwise back up over this component. */
				while (*(--new_path) != '/') ;
			/* Safe sex check. */
			if (strlen(path) + n >= PATH_MAX - 2) {
				errno = ENAMETOOLONG;
				return NULL;
			}
			/* Insert symlink contents into path. */
			strcat(link_path, path);
			strcpy(copy_path, link_path);
			path = copy_path;
		}
#endif				/* S_IFLNK */
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
