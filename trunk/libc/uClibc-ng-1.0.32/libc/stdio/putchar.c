/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


#undef putchar
#ifdef __DO_UNLOCKED

#undef putchar_unlocked
int putchar_unlocked(int c)
{
	register FILE *stream = stdout;

	return __PUTC_UNLOCKED_MACRO(c, stream);
}

#ifndef __UCLIBC_HAS_THREADS__
strong_alias(putchar_unlocked,putchar)
#endif

#elif defined __UCLIBC_HAS_THREADS__

int putchar(int c)
{
	register FILE *stream = stdout;

	if (stream->__user_locking != 0) {
		return __PUTC_UNLOCKED_MACRO(c, stream);
	} else {
		int retval;
		__STDIO_ALWAYS_THREADLOCK(stream);
		retval = __PUTC_UNLOCKED_MACRO(c, stream);
		__STDIO_ALWAYS_THREADUNLOCK(stream);
		return retval;
	}
}

#endif
