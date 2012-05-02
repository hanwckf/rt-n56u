/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifdef __STDIO_BUFFERS

/* Commit the data in the write buffer.
 * Returns 0 on success, >0 (pending) on failure.
 * Side effects are those of _stdio_WRITE
 */

size_t __stdio_wcommit(register FILE * __restrict stream)
{
	size_t bufsize;

	__STDIO_STREAM_VALIDATE(stream);

	if ((bufsize = __STDIO_STREAM_BUFFER_WUSED(stream)) != 0) {
		stream->__bufpos = stream->__bufstart;
		__stdio_WRITE(stream, stream->__bufstart, bufsize);
	}

	return __STDIO_STREAM_BUFFER_WUSED(stream);
}

#endif
