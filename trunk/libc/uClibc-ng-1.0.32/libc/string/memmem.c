/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef __USE_GNU
void *memmem(const void *haystack, size_t haystacklen,
		     const void *needle, size_t needlelen)
{
	register const char *ph;
	register const char *pn;
	const char *plast;
	size_t n;

	if (needlelen == 0) {
		return (void *) haystack;
	}

	if (haystacklen >= needlelen) {
		ph = (const char *) haystack;
		pn = (const char *) needle;
		plast = ph + (haystacklen - needlelen);

		do {
			n = 0;
			while (ph[n] == pn[n]) {
				if (++n == needlelen) {
					return (void *) ph;
				}
			}
		} while (++ph <= plast);
	}

	return NULL;
}
#endif
