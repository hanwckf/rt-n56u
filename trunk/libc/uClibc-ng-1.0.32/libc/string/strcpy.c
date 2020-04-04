/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wstrcpy wcscpy
#else
# define Wstrcpy strcpy
#endif

Wchar *Wstrcpy(Wchar * __restrict s1, const Wchar * __restrict s2)
{
	register Wchar *s = s1;

	while ( (*s++ = *s2++) != 0 );

	return s1;
}

#ifndef WANT_WIDE
libc_hidden_def(strcpy)
#endif
