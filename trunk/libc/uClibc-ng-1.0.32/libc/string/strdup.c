/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"
#include <stdlib.h>

#ifdef WANT_WIDE
# define Wstrdup wcsdup
# define Wstrlen wcslen
#else
# define Wstrdup strdup
# define Wstrlen strlen
#endif

Wchar *Wstrdup(register const Wchar *s1)
{
	register Wchar *s;
	register size_t l = (Wstrlen(s1) + 1) * sizeof(Wchar);

	if ((s = malloc(l)) != NULL) {
		memcpy(s, s1, l);
	}

	return s;
}

#ifndef WANT_WIDE
libc_hidden_weak(strdup)
#endif
