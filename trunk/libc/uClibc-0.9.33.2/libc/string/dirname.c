/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"
#include <libgen.h>

char *dirname(char *path)
{
	static const char null_or_empty_or_noslash[] = ".";
	register char *s;
	register char *last;
	char *first;

	last = s = path;

	if (s != NULL) {

	LOOP:
		while (*s && (*s != '/')) ++s;
		first = s;
		while (*s == '/') ++s;
		if (*s) {
			last = first;
			goto LOOP;
		}

		if (last == path) {
			if (*last != '/') {
				goto DOT;
			}
			if ((*++last == '/') && (last[1] == 0)) {
				++last;
			}
		}
		*last = 0;
		return path;
	}
 DOT:
	return (char *) null_or_empty_or_noslash;
}
