/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wstrstr wcsstr
#else
# define Wstrstr strstr
#endif

/* NOTE: This is the simple-minded O(len(s1) * len(s2)) worst-case approach. */

Wchar *Wstrstr(const Wchar *s1, const Wchar *s2)
{
	register const Wchar *s = s1;
	register const Wchar *p = s2;

	do {
		if (!*p) {
			return (Wchar *) s1;;
		}
		if (*p == *s) {
			++p;
			++s;
		} else {
			p = s2;
			if (!*s) {
				return NULL;
			}
			s = ++s1;
		}
	} while (1);
}
#ifndef WANT_WIDE
libc_hidden_def(strstr)
#elif defined __UCLIBC_SUSV3_LEGACY__
strong_alias(wcsstr,wcswcs)
#endif
