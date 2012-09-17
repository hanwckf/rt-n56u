/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_string.h"

#if 0 /*def WANT_WIDE*/
# define Wstrtok wcstok
# define Wstrtok_r wcstok_r
#else
# define Wstrtok strtok
# define Wstrtok_r strtok_r
#endif


Wchar *Wstrtok(Wchar * __restrict s1, const Wchar * __restrict s2)
{
	static Wchar *next_start;	/* Initialized to 0 since in bss. */
	return Wstrtok_r(s1, s2, &next_start);
}
libc_hidden_def(Wstrtok)
