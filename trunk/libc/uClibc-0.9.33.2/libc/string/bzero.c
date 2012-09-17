/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef __UCLIBC_SUSV3_LEGACY__
void bzero(void *s, size_t n)
{
#if 1
	(void)memset(s, 0, n);
#else
	register unsigned char *p = s;

	while (n) {
		*p++ = 0;
		--n;
	}
#endif
}
#endif
