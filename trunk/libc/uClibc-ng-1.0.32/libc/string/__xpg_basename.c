/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <libgen.h>

char *__xpg_basename(register char *path)
{
	static const char null_or_empty[] = ".";
	char *first;
	register char *last;

	first = (char *) null_or_empty;

	if (path && *path) {
		first = path;
		last = path - 1;

		do {
			if ((*path != '/') && (path > ++last)) {
				last = first = path;
			}
		} while (*++path);

		if (*first == '/') {
			last = first;
		}
		last[1] = 0;
	}

	return first;
}
#ifndef __USE_GNU
# undef basename
weak_alias(__xpg_basename,basename)
#endif
