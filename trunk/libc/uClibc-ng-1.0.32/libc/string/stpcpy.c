/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#ifdef WANT_WIDE
# define Wstpcpy wcpcpy
#else
# undef stpcpy
# define Wstpcpy stpcpy
#endif

Wchar *Wstpcpy(register Wchar * __restrict s1, const Wchar * __restrict s2)
{
	while ( (*s1++ = *s2++) != 0 );

	return s1 - 1;
}

#ifndef WANT_WIDE
libc_hidden_def(stpcpy)
#endif
