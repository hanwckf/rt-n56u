/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wstrcspn wcscspn
#else
# define Wstrcspn strcspn
#endif

size_t Wstrcspn(const Wchar *s1, const Wchar *s2)
{
	register const Wchar *s;
	register const Wchar *p;

	for ( s=s1 ; *s ; s++ ) {
		for ( p=s2 ; *p ; p++ ) {
			if (*p == *s) goto done;
		}
	}
 done:
	return s - s1;
}

#ifndef WANT_WIDE
libc_hidden_def(strcspn)
#endif
