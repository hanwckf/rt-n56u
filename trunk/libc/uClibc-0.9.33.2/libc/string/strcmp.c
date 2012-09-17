/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wstrcmp wcscmp
# define Wstrcoll wcscoll
#else
# define Wstrcmp strcmp
# define Wstrcoll strcoll
#endif

int Wstrcmp(register const Wchar *s1, register const Wchar *s2)
{
#ifdef WANT_WIDE
	while (*((Wuchar *)s1) == *((Wuchar *)s2)) {
		if (!*s1++) {
			return 0;
		}
		++s2;
	}

	return (*((Wuchar *)s1) < *((Wuchar *)s2)) ? -1 : 1;
#else
	int r;

	while (((r = ((int)(*((Wuchar *)s1))) - *((Wuchar *)s2++))
			== 0) && *s1++);

	return r;
#endif
}
libc_hidden_def(Wstrcmp)

#ifndef __UCLIBC_HAS_LOCALE__
strong_alias(Wstrcmp,Wstrcoll)
libc_hidden_def(Wstrcoll)
#endif
