/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wmemcmp wmemcmp
#else
# define Wmemcmp memcmp
#endif

int Wmemcmp(const Wvoid *s1, const Wvoid *s2, size_t n)
{
	register const Wuchar *r1 = (const Wuchar *) s1;
	register const Wuchar *r2 = (const Wuchar *) s2;

#ifdef WANT_WIDE
	while (n && (*r1 == *r2)) {
		++r1;
		++r2;
		--n;
	}

	return (n == 0) ? 0 : ((*r1 < *r2) ? -1 : 1);
#else
	int r = 0;

	while (n-- && ((r = ((int)(*r1++)) - *r2++) == 0));

	return r;
#endif
}

#ifndef WANT_WIDE
libc_hidden_def(memcmp)
# ifdef __UCLIBC_SUSV3_LEGACY__
strong_alias(memcmp,bcmp)
# endif
#endif
