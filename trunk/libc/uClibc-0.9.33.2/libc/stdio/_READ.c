/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


/* Given a reading stream without its end-of-file indicator set and
 * with no buffered input or ungots, read at most 'bufsize' bytes
 * into 'buf' (which may be the stream's __bufstart).
 * If a read error occurs, set the stream's error indicator.
 * If EOF is encountered, set the stream's end-of-file indicator.
 *
 * Returns the number of bytes read, even in EOF and error cases.
 *
 * Notes:
 *   Calling with bufsize == 0 is NOT permitted (unlike __stdio_WRITE).
 *   NOT THREADSAFE!  Assumes stream already locked if necessary.
 */

size_t attribute_hidden __stdio_READ(register FILE *stream,
					unsigned char *buf, size_t bufsize)
{
	ssize_t rv = 0;

	__STDIO_STREAM_VALIDATE(stream);
	assert(stream->__filedes >= -1);
	assert(__STDIO_STREAM_IS_READING(stream));
	assert(!__STDIO_STREAM_BUFFER_RAVAIL(stream)); /* Buffer must be empty. */
	assert(!(stream->__modeflags & __FLAG_UNGOT));
	assert(bufsize);

	if (!__FEOF_UNLOCKED(stream)) {
		if (bufsize > SSIZE_MAX) {
			bufsize = SSIZE_MAX;
		}

#ifdef __UCLIBC_MJN3_ONLY__
#warning EINTR?
#endif
/* 	RETRY: */
		if ((rv = __READ(stream, (char *) buf, bufsize)) <= 0) {
			if (rv == 0) {
				__STDIO_STREAM_SET_EOF(stream);
			} else {
/* 				if (errno == EINTR) goto RETRY; */
				__STDIO_STREAM_SET_ERROR(stream);
				rv = 0;
			}
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Make custom stream read return check optional.
#endif
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
		} else {
			assert(rv <= bufsize);
			if (rv > bufsize) {	/* Read more than bufsize! */
				abort();
			}
#endif
		}
	}

	return rv;
}
