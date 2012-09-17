/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wstrchr wcschr
#else
# define Wstrchr strchr
#endif

Wchar *Wstrchr(register const Wchar *s, Wint c)
{
	do {
		if (*s == ((Wchar)c)) {
			return (Wchar *) s;	/* silence the warning */
		}
	} while (*s++);

	return NULL;
}
#ifndef WANT_WIDE
libc_hidden_def(strchr)
# ifdef __UCLIBC_SUSV3_LEGACY__
weak_alias(strchr,index)
# endif
#endif
