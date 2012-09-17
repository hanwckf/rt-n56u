/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


#undef getchar
#ifdef __DO_UNLOCKED

/* the only use of the hidden getchar_unlocked is in gets.c */
#undef getchar_unlocked
int getchar_unlocked(void)
{
	register FILE *stream = stdin;

	return __GETC_UNLOCKED_MACRO(stream);
}
libc_hidden_def(getchar_unlocked)

#ifndef __UCLIBC_HAS_THREADS__
strong_alias(getchar_unlocked,getchar)
#endif

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
