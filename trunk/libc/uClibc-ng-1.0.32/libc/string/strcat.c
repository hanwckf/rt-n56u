/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wstrcat wcscat
#else
# define Wstrcat strcat
#endif

Wchar *Wstrcat(Wchar * __restrict s1, register const Wchar * __restrict s2)
{
	register Wchar *s = s1;

	while (*s++);
	--s;
	while ((*s++ = *s2++) != 0);

	return s1;
}
libc_hidden_def(Wstrcat)
