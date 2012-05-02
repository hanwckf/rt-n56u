/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifdef __DO_UNLOCKED

weak_alias(__fputwc_unlocked,fputwc_unlocked);
weak_alias(__fputwc_unlocked,putwc_unlocked);
#ifndef __UCLIBC_HAS_THREADS__
weak_alias(__fputwc_unlocked,fputwc);
weak_alias(__fputwc_unlocked,putwc);
#endif

wint_t __fputwc_unlocked(wchar_t wc, FILE *stream)
{
	return _wstdio_fwrite(&wc, 1, stream) ? wc : WEOF;
}

#elif defined __UCLIBC_HAS_THREADS__

weak_alias(fputwc,putwc);

wint_t fputwc(wchar_t wc, register FILE *stream)
{
	wint_t retval;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	retval = __fputwc_unlocked(wc, stream);

	__STDIO_AUTO_THREADUNLOCK(stream);

	return retval;
}

#endif
