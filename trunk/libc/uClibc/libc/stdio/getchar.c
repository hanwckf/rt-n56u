/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#undef getchar_unlocked
#undef getchar

#ifdef __DO_UNLOCKED

weak_alias(__getchar_unlocked,getchar_unlocked);
#ifndef __UCLIBC_HAS_THREADS__
weak_alias(__getchar_unlocked,getchar);
#endif

int __getchar_unlocked(void)
{
	register FILE *stream = stdin;

	return __GETC_UNLOCKED_MACRO(stream);
}

#elif defined __UCLIBC_HAS_THREADS__

int getchar(void)
{
	register FILE *stream = stdin;

	if (stream->__user_locking != 0) {
		return __GETC_UNLOCKED_MACRO(stream);
	} else {
		int retval;
		__STDIO_ALWAYS_THREADLOCK(stream);
		retval = __GETC_UNLOCKED_MACRO(stream);
		__STDIO_ALWAYS_THREADUNLOCK(stream);
		return retval;
	}
}

#endif
