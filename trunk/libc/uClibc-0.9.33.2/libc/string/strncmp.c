/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wstrncmp wcsncmp
#else
# define Wstrncmp strncmp
#endif

int Wstrncmp(register const Wchar *s1, register const Wchar *s2, size_t n)
{
#ifdef WANT_WIDE
	while (n && (*((Wuchar *)s1) == *((Wuchar *)s2))) {
		if (!*s1++) {
			return 0;
		}
		++s2;
		--n;
	}

	return (n == 0) ? 0 : (*((Wuchar *)s1) - *((Wuchar *)s2));
#else
	int r = 0;

	while (n--
		   && ((r = ((int)(*((unsigned char *)s1))) - *((unsigned char *)s2++))
			== 0)
		   && *s1++);

	return r;
#endif
}
#ifndef WANT_WIDE
libc_hidden_weak(strncmp)
#endif
