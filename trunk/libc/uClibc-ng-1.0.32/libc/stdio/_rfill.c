/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifdef __STDIO_BUFFERS

/* Read some data into the buffer.
 * Returns number of bytes read into the buffer.
 * If 0 is returned, then either EOF or ERROR.
 * Side effects are those of _stdio_READ.
 */

size_t attribute_hidden __stdio_rfill(register FILE *__restrict stream)
{
	size_t rv;

	__STDIO_STREAM_VALIDATE(stream);
	assert(stream->__filedes >= -2);
	assert(__STDIO_STREAM_IS_READING(stream));
	assert(!__STDIO_STREAM_BUFFER_RAVAIL(stream)); /* Buffer must be empty. */
	assert(__STDIO_STREAM_BUFFER_SIZE(stream));	/* Must have a buffer. */
	assert(!(stream->__modeflags & __FLAG_UNGOT));
#ifdef __UCLIBC_HAS_STDIO_GETC_MACRO__
	assert(stream->__bufgetc_u == stream->__bufstart);
#endif

	rv = __stdio_READ(stream, stream->__bufstart,
					  stream->__bufend - stream->__bufstart);
	stream->__bufpos = stream->__bufstart;
	stream->__bufread = stream->__bufstart + rv;

	__STDIO_STREAM_VALIDATE(stream);
	return rv;
}

#endif
