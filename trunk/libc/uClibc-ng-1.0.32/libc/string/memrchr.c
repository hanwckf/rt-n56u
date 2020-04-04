/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef __USE_GNU
void *memrchr(const void *s, int c, size_t n)
{
	register const unsigned char *r;

	r = ((unsigned char *)s) + ((size_t) n);

	while (n) {
		if (*--r == ((unsigned char)c)) {
			return (void *) r;	/* silence the warning */
		}
		--n;
	}

	return NULL;
}

libc_hidden_def(memrchr)
#endif
