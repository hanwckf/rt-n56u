/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef __USE_GNU
void *rawmemchr(const void *s, int c)
{
	register const unsigned char *r = s;

	while (*r != ((unsigned char)c)) ++r;

	return (void *) r;	/* silence the warning */
}
libc_hidden_def(rawmemchr)
#endif
