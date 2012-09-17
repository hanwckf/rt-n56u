/* Copyright (C) 1992-1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  
   */

/* Modified for uClibc by Erik Andersen
   */

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include "dirstream.h"

int scandir(const char *dir, struct dirent ***namelist, 
	int (*selector) (const struct dirent *),
	int (*compar) (const void *, const void *))
{
    DIR *dp = opendir (dir);
    struct dirent *current;
    struct dirent **names = NULL;
    size_t names_size = 0, pos;
    int save;

    if (dp == NULL)
	return -1;

    save = errno;
    __set_errno (0);

    pos = 0;
    while ((current = readdir (dp)) != NULL)
	if (selector == NULL || (*selector) (current))
	{
	    struct dirent *vnew;
	    size_t dsize;

	    /* Ignore errors from selector or readdir */
	    __set_errno (0);

	    if (unlikely(pos == names_size))
	    {
		struct dirent **new;
		if (names_size == 0)
		    names_size = 10;
		else
		    names_size *= 2;
		new = (struct dirent **) realloc (names, names_size * sizeof (struct dirent *));
		if (new == NULL)
		    break;
		names = new;
	    }

	    dsize = &current->d_name[_D_ALLOC_NAMLEN (current)] - (char *) current;
	    vnew = (struct dirent *) malloc (dsize);
	    if (vnew == NULL)
		break;

	    names[pos++] = (struct dirent *) memcpy (vnew, current, dsize);
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
	qsort (names, pos, sizeof (struct dirent *), compar);
    *namelist = names;
    return pos;
}
