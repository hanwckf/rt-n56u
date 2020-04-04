/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef __USE_GNU

#ifdef WANT_WIDE
# define Wstrchrnul wcschrnul
#else
# define Wstrchrnul strchrnul
#endif

Wchar *Wstrchrnul(register const Wchar *s, Wint c)
{
	--s;
	while (*++s && (*s != ((Wchar)c)));
	return (Wchar *) s;
}
# ifndef WANT_WIDE
libc_hidden_def(strchrnul)
# endif
#endif
