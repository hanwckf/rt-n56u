/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wstrrchr wcsrchr
#else
# define Wstrrchr strrchr
#endif

Wchar *Wstrrchr(register const  Wchar *s, Wint c)
{
	register const Wchar *p;

	p = NULL;
	do {
		if (*s == (Wchar) c) {
			p = s;
		}
	} while (*s++);

	return (Wchar *) p;			/* silence the warning */
}
#ifndef WANT_WIDE
libc_hidden_weak(strrchr)
# ifdef __UCLIBC_SUSV3_LEGACY__
weak_alias(strrchr,rindex)
# endif
#endif
