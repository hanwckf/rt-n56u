/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

int puts(register const char * __restrict s)
{
	register FILE *stream = stdout; /* This helps bcc optimize. */
	int n;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	/* Note: Don't try to optimize by switching to FBF until the newline.
	 * If the string itself contained a newline a write error occurred,
	 * then we could have a newline in the buffer of an LBF stream. */

	/* Note: Nonportable as fputs need only return nonnegative on success. */
	if ((n = __fputs_unlocked(s, stream)) != EOF) {
		++n;
		if (__fputc_unlocked('\n', stream) == EOF) {
			n = EOF;
		}
	}

	__STDIO_AUTO_THREADUNLOCK(stream);

	return n;
}
