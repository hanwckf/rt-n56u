/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <string.h>

#ifdef __UCLIBC_SUSV3_LEGACY__
void bcopy(const void *s2, void *s1, size_t n)
{
#if 1
	memmove(s1, s2, n);
#else
	register char *s;
	register const char *p;

	s = s1;
	p = s2;
	if (p >= s) {
		while (n) {
			*s++ = *p++;
			--n;
		}
	} else {
		while (n) {
			--n;
			s[n] = p[n];
		}
	}
#endif
}
#endif
