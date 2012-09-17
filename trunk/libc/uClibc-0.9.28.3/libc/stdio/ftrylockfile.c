/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: What should this return when not threading?
#endif

int ftrylockfile(FILE *stream)
{
	__STDIO_STREAM_VALIDATE(stream);

	return __STDIO_ALWAYS_THREADTRYLOCK_CANCEL_UNSAFE(stream);
}
