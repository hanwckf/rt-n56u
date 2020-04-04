/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"



#ifdef __DO_UNLOCKED

int fputws_unlocked(const wchar_t *__restrict ws,
					  register FILE *__restrict stream)
{
	size_t n = wcslen(ws);

	return (_wstdio_fwrite(ws, n, stream) == n) ? 0 : -1;
}
libc_hidden_def(fputws_unlocked)

#ifndef __UCLIBC_HAS_THREADS__
strong_alias(fputws_unlocked,fputws)
libc_hidden_def(fputws)
#endif

#elif defined __UCLIBC_HAS_THREADS__

int fputws(const wchar_t *__restrict ws, register FILE *__restrict stream)
{
	int retval;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	retval = fputws_unlocked(ws, stream);

	__STDIO_AUTO_THREADUNLOCK(stream);

	return retval;
}
libc_hidden_def(fputws)

#endif
