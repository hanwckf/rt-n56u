/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


void rewind(register FILE *stream)
{
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	__STDIO_STREAM_CLEAR_ERROR(stream);	/* Clear the error indicator */
	fseek(stream, 0L, SEEK_SET); /* first since fseek could set it. */

	__STDIO_AUTO_THREADUNLOCK(stream);
}
libc_hidden_def(rewind)
