/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#undef feof
#ifdef __DO_UNLOCKED

#undef feof_unlocked
int feof_unlocked(register FILE *stream)
{
	__STDIO_STREAM_VALIDATE(stream);

	return __FEOF_UNLOCKED(stream);
}

#ifndef __UCLIBC_HAS_THREADS__
strong_alias(feof_unlocked,feof)
#endif

#elif defined __UCLIBC_HAS_THREADS__

int feof(register FILE *stream)
{
	int retval;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	__STDIO_STREAM_VALIDATE(stream);

	retval = __FEOF_UNLOCKED(stream);

	__STDIO_AUTO_THREADUNLOCK(stream);

	return retval;
}

#endif
