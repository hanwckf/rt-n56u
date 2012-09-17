/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#undef clearerr
#ifdef __DO_UNLOCKED

#undef clearerr_unlocked
void clearerr_unlocked(register FILE *stream)
{
	__STDIO_STREAM_VALIDATE(stream);

	__CLEARERR_UNLOCKED(stream);
}

#ifndef __UCLIBC_HAS_THREADS__
strong_alias(clearerr_unlocked,clearerr)
#endif

#elif defined __UCLIBC_HAS_THREADS__

void clearerr(register FILE *stream)
{
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	__STDIO_STREAM_VALIDATE(stream);

	__CLEARERR_UNLOCKED(stream);

	__STDIO_AUTO_THREADUNLOCK(stream);
}

#endif
