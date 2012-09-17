/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"
#include <stdlib.h>


char *strndup(register const char *s1, size_t n)
{
	register char *s;

	n = strnlen(s1,n);			/* Avoid problems if s1 not nul-terminated. */

    if ((s = malloc(n + 1)) != NULL) {
		memcpy(s, s1, n);
		s[n] = 0;
	}

	return s;
}
libc_hidden_def(strndup)
