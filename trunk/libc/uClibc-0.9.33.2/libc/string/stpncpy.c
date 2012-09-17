/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wstpncpy wcpncpy
#else
# define Wstpncpy stpncpy
#endif

Wchar *Wstpncpy(register Wchar * __restrict s1,
				register const Wchar * __restrict s2,
				size_t n)
{
	Wchar *s = s1;
	const Wchar *p = s2;

	while (n) {
		if ((*s = *s2) != 0) s2++; /* Need to fill tail with 0s. */
		++s;
		--n;
	}
	return s1 + (s2 - p);
}
