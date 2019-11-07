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

size_t attribute_hidden __stdio_WRITE(register FILE *stream,
					 register const unsigned char *buf, size_t bufsize)
{
	size_t todo;
	ssize_t rv, stodo;

	__STDIO_STREAM_VALIDATE(stream);
	assert(stream->__filedes >= -2);
	assert(__STDIO_STREAM_IS_WRITING(stream));
	assert(!__STDIO_STREAM_BUFFER_WUSED(stream)); /* Buffer must be empty. */

	todo = bufsize;

	while (todo != 0) {
		stodo = (todo <= SSIZE_MAX) ? todo : SSIZE_MAX;
		rv = __WRITE(stream, (char *) buf, stodo);
		if (rv >= 0) {
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
			assert(rv <= stodo);
			if (rv > stodo) {	/* Wrote more than stodo! */
/* 				abort(); */
			}
#endif
			todo -= rv;
			buf += rv;
		} else {

			__STDIO_STREAM_SET_ERROR(stream);

			/* We buffer data on "transient" errors, but discard it
			 * on "hard" ones. Example of a hard error:
			 *
			 * close(fileno(stdout));
			 * printf("Hi there 1\n"); // EBADF
			 * dup2(good_fd, fileno(stdout));
			 * printf("Hi there 2\n"); // buffers new data
			 *
			 * This program should not print "Hi there 1" to good_fd.
			 * The rationale is that the caller of writing operation
			 * should check for error and act on it.
			 * If he didn't, then future users of the stream
			 * have no idea what to do.
			 * It's least confusing to at least not burden them with
			 * some hidden buffered crap in the buffer.
			 */
			if (errno != EINTR && errno != EAGAIN) {
				/* do we have other "soft" errors? */
				bufsize -= todo;
				break;
			}
#ifdef __STDIO_BUFFERS
			stodo = __STDIO_STREAM_BUFFER_SIZE(stream);
			if (stodo != 0) {
				unsigned char *s;

				if (stodo > todo) {
					stodo = todo;
				}

				s = stream->__bufstart;

				do {
					*s = *buf;
					if ((*s == '\n')
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

			bufsize -= todo;
			break;
		}
	}

	__STDIO_STREAM_VALIDATE(stream);
	return bufsize;
}
