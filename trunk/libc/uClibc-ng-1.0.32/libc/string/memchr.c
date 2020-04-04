/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wmemchr wmemchr
#else
# undef memchr
# define Wmemchr memchr
#endif

Wvoid *Wmemchr(const Wvoid *s, Wint c, size_t n)
{
	register const Wuchar *r = (const Wuchar *) s;

	while (n) {
		if (*r == ((Wuchar)c)) {
			return (Wvoid *) r;	/* silence the warning */
		}
		++r;
		--n;
	}

	return NULL;
}

libc_hidden_def(Wmemchr)
