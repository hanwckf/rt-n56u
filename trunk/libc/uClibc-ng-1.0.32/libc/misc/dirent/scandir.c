/*
 * Copyright (C) 2000-2011 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "dirstream.h"

#ifndef __SCANDIR
# define __SCANDIR scandir
# define __DIRENT_TYPE struct dirent
# define __READDIR readdir
#endif

int __SCANDIR(const char *dir, __DIRENT_TYPE ***namelist,
	int (*selector) (const __DIRENT_TYPE *),
	int (*compar) (const __DIRENT_TYPE **, const __DIRENT_TYPE **))
{
    DIR *dp = opendir (dir);
    __DIRENT_TYPE *current;
    __DIRENT_TYPE **names = NULL;
    size_t names_size = 0, pos;
    int save;

    if (dp == NULL)
	return -1;

    save = errno;
    __set_errno (0);

    pos = 0;
    while ((current = __READDIR (dp)) != NULL) {
	int use_it = selector == NULL;

	if (! use_it)
	{
	    use_it = (*selector) (current);
	    /* The selector function might have changed errno.
	     * It was zero before and it need to be again to make
	     * the latter tests work.  */
	    if (! use_it)
		__set_errno (0);
	}
	if (use_it)
	{
	    __DIRENT_TYPE *vnew;
	    size_t dsize;

	    /* Ignore errors from selector or readdir */
	    __set_errno (0);

	    if (unlikely(pos == names_size))
	    {
		__DIRENT_TYPE **new;
		if (names_size == 0)
		    names_size = 10;
		else
		    names_size *= 2;
		new = (__DIRENT_TYPE **) realloc (names,
					names_size * sizeof (__DIRENT_TYPE *));
		if (new == NULL)
		    break;
		names = new;
	    }

	    dsize = &current->d_name[_D_ALLOC_NAMLEN(current)] - (char*)current;
	    vnew = (__DIRENT_TYPE *) malloc (dsize);
	    if (vnew == NULL)
		break;

	    names[pos++] = (__DIRENT_TYPE *) memcpy (vnew, current, dsize);
	}
    }

    if (unlikely(errno != 0))
    {
	save = errno;
	closedir (dp);
	while (pos > 0)
	    free (names[--pos]);
	free (names);
	__set_errno (save);
	return -1;
    }

    closedir (dp);
    __set_errno (save);

    /* Sort the list if we have a comparison function to sort with.  */
    if (compar != NULL)
	qsort (names, pos, sizeof (__DIRENT_TYPE *), (comparison_fn_t) compar);
    *namelist = names;
    return pos;
}
#if __WORDSIZE == 64
strong_alias_untyped(scandir,scandir64)
#endif
