/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wmemset wmemset
#else
# undef memset
# define Wmemset memset
#endif

Wvoid *Wmemset(Wvoid *s, Wint c, size_t n)
{
	register Wuchar *p = (Wuchar *) s;

	while (n) {
		*p++ = (Wuchar) c;
		--n;
	}

	return s;
}

#ifndef WANT_WIDE
libc_hidden_def(memset)
#endif
