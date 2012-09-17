/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <stdio_ext.h>


/* Not threadsafe. */

/* Notes:
 *   When setting the locking mode, glibc returns the previous setting.
 *   glibc treats invalid locking_mode args as FSETLOCKING_INTERNAL.
 */

int __fsetlocking(FILE *stream, int locking_mode)
{
#ifdef __UCLIBC_HAS_THREADS__
	int current = 1 + (stream->__user_locking & 1);

	/* Check constant assumptions.  We can't test at build time
	 * since these are enums. */
	assert((FSETLOCKING_QUERY == 0) && (FSETLOCKING_INTERNAL == 1)
		   && (FSETLOCKING_BYCALLER == 2));

	__STDIO_STREAM_VALIDATE(stream);
	assert(((unsigned int) locking_mode) <= 2);

	if (locking_mode != FSETLOCKING_QUERY) {
		stream->__user_locking = ((locking_mode == FSETLOCKING_BYCALLER)
								  ? 1
								  : _stdio_user_locking); /* 0 or 2 */
		__STDIO_STREAM_VALIDATE(stream);
	}

	return current;
#else
	__STDIO_STREAM_VALIDATE(stream);
	assert(((unsigned int) locking_mode) <= 2);

	return FSETLOCKING_INTERNAL;
#endif
}
libc_hidden_def(__fsetlocking)
