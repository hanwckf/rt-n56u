/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef __USE_BSD


char *strsep(char ** __restrict s1, const char * __restrict s2)
{
	register char *s = *s1;
	register char *p;

#if 1
	p = NULL;
	if (s && *s && (p = strpbrk(s, s2))) {
		*p++ = 0;
	}
#else
	if (s && *s && *(p = s + strcspn(s, s2))) {
		*p++ = 0;
	} else {
		p = NULL;
	}
#endif
	*s1 = p;
	return s;
}
libc_hidden_def(strsep)
#endif
