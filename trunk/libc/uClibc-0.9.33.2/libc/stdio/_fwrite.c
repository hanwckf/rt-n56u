/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


#ifdef __STDIO_BUFFERS

/* Either buffer data or (commit buffer if necessary and) write. */

size_t attribute_hidden __stdio_fwrite(const unsigned char * __restrict buffer,
					  size_t bytes,
					  register FILE * __restrict stream)
{
	size_t pending;
	const unsigned char *p;

	__STDIO_STREAM_VALIDATE(stream);
	assert(__STDIO_STREAM_IS_WRITING(stream));
	assert(buffer);
	assert(bytes);

	if (!__STDIO_STREAM_IS_NBF(stream)) { /* FBF or LBF. */
#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: Try to consolidate some of the code?
#endif
		if (__STDIO_STREAM_IS_FAKE_VSNPRINTF(stream)) {
			pending = __STDIO_STREAM_BUFFER_WAVAIL(stream);
			if (pending > bytes) {
				pending = bytes;
			}
			memcpy(stream->__bufpos, buffer, pending);
			stream->__bufpos += pending;
			__STDIO_STREAM_VALIDATE(stream);
			return bytes;
		}

/* 	RETRY: */
		if (bytes <= __STDIO_STREAM_BUFFER_WAVAIL(stream)) {
			memcpy(stream->__bufpos, buffer, bytes);
			stream->__bufpos += bytes;
			if (__STDIO_STREAM_IS_LBF(stream)
				&& memrchr(buffer, '\n', bytes)	/* Search backwards. */
				) {
				if ((pending = __STDIO_COMMIT_WRITE_BUFFER(stream)) > 0) {
					if (pending > bytes) {
						pending = bytes;
					}
					buffer += (bytes - pending);
					if ((p = memchr(buffer, '\n', pending)) != NULL) {
						pending = (buffer + pending) - p;
						bytes -= pending;
						stream->__bufpos -= pending;
					}
				}
			}
			__STDIO_STREAM_VALIDATE(stream);
			return bytes;
		}
		/* FBF or LBF and not enough space in buffer. */
		if (__STDIO_STREAM_BUFFER_WUSED(stream)) { /* Buffered data. */
			if (__STDIO_COMMIT_WRITE_BUFFER(stream)) { /* Commit failed! */
				return 0;
			}
#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: Do we want to try again if data now fits in buffer?
#endif
/* 			goto RETRY; */
		}
	}

	return __stdio_WRITE(stream, buffer, bytes);
}

#endif
