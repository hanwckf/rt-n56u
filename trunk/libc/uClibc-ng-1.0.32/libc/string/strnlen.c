/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef __USE_GNU

#ifdef WANT_WIDE
# define Wstrnlen wcsnlen
#else
# define Wstrnlen strnlen
#endif

size_t Wstrnlen(const Wchar *s, size_t max)
{
	register const Wchar *p = s;

	while (max && *p) {
		++p;
		--max;
	}

	return p - s;
}

libc_hidden_def(Wstrnlen)
#endif
