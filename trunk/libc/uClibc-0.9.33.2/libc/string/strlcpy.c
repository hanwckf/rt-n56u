/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wstrlcpy __wcslcpy
# define Wstrxfrm wcsxfrm
#else
# define Wstrlcpy strlcpy
# define Wstrxfrm strxfrm
#endif

/* OpenBSD function:
 * Copy at most n-1 chars from src to dst and nul-terminate dst.
 * Returns strlen(src), so truncation occurred if the return value is >= n. */

size_t Wstrlcpy(register Wchar *__restrict dst,
				  register const Wchar *__restrict src,
				  size_t n)
{
	const Wchar *src0 = src;
	Wchar dummy[1];

	if (!n) {
		dst = dummy;
	} else {
		--n;
	}

	while ((*dst = *src) != 0) {
		if (n) {
			--n;
			++dst;
		}
		++src;
	}

	return src - src0;
}
#ifndef WANT_WIDE
libc_hidden_def(strlcpy)
#endif

#ifndef __UCLIBC_HAS_LOCALE__
strong_alias(Wstrlcpy,Wstrxfrm)
#endif
