/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/* TODO: According to SUSv3 should return EBADF if invalid stream. */

int fwide(register FILE *stream, int mode)
{
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	__STDIO_STREAM_VALIDATE(stream);

	if (mode && !(stream->__modeflags & (__FLAG_WIDE|__FLAG_NARROW))) {
		stream->__modeflags |= ((mode > 0) ? __FLAG_WIDE : __FLAG_NARROW);
	}

	mode = (stream->__modeflags & __FLAG_WIDE)
		- (stream->__modeflags & __FLAG_NARROW);

	assert((stream->__modeflags & (__FLAG_WIDE|__FLAG_NARROW))
		   != (__FLAG_WIDE|__FLAG_NARROW));
	__STDIO_AUTO_THREADUNLOCK(stream);

	return mode;
}
