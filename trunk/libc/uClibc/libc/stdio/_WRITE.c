/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/* Given a writing stream with no buffered output, write the
 * data in 'buf' (which may be the stream's bufstart) of size
 * 'bufsize' to the stream.  If a write error occurs, set the
 * stream's error indicator and (if buffering) buffer as much
 * data as possible (FBF) or only up to '\n' (LBF) to implement
 * "as if fputc()" clause in the standard.
 *
 * Returns the number of bytes written and/or buffered.
 *
 * Notes:
 *   Calling with bufsize == 0 is permitted, and buf is ignored in
 *     that case.
 *   We implement fflush() by setting bufpos to bufstart and passing
 *     bufstart as the buf arg.  If there is a write error, the
 *     unwritten buffered data will simply be moved to the beginning
 *     of the buffer.  Since the data obviously fits in the buffer
 *     and since there will be no '\n' chars in the buffer in the LBF
 *     case, no data will be lost.
 *   NOT THREADSAFE!  Assumes stream already locked if necessary.
 */

size_t __stdio_WRITE(register FILE *stream,
					 register const unsigned char *buf, size_t bufsize)
{
	size_t todo;
	ssize_t rv, stodo;

	__STDIO_STREAM_VALIDATE(stream);
	assert(stream->__filedes >= -1);
	assert(__STDIO_STREAM_IS_WRITING(stream));
	assert(!__STDIO_STREAM_BUFFER_WUSED(stream)); /* Buffer must be empty. */

	todo = bufsize;

	do {
		if (todo == 0) {		/* Done? */
			__STDIO_STREAM_VALIDATE(stream);
			return bufsize;
		}
		stodo = (todo <= SSIZE_MAX) ? todo : SSIZE_MAX;
		if ((rv = __WRITE(stream, (const char*)buf, stodo)) >= 0) {
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Make custom stream write return check optional.
#endif
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
			assert(rv <= stodo);
			if (rv > stodo) {	/* Wrote more than stodo! */
/* 				abort(); */
			}
#endif
			todo -= rv;
			buf += rv;
		} else
#ifdef __UCLIBC_MJN3_ONLY__
#warning EINTR?
#endif
/* 		if (errno != EINTR) */
		{
			__STDIO_STREAM_SET_ERROR(stream);

#ifdef __STDIO_BUFFERS
			if ((stodo = __STDIO_STREAM_BUFFER_SIZE(stream)) != 0) {
				unsigned char *s;

				if (stodo > todo) {
					stodo = todo;
				}

				s  = stream->__bufstart;

				do {
					if (((*s = *buf) == '\n')
						&& __STDIO_STREAM_IS_LBF(stream)
						) {
						break;
					}
					++s;
					++buf;
				} while (--stodo);

				stream->__bufpos = s;

				todo -= (s - stream->__bufstart);
			}
#endif /* __STDIO_BUFFERS */

			__STDIO_STREAM_VALIDATE(stream);
			return bufsize - todo;
		}
	} while (1);
}
