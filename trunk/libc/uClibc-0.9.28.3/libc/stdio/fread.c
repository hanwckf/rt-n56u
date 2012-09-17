/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifdef __DO_UNLOCKED

weak_alias(__fread_unlocked,fread_unlocked);
#ifndef __UCLIBC_HAS_THREADS__
weak_alias(__fread_unlocked,fread);
#endif

size_t __fread_unlocked(void * __restrict ptr, size_t size, size_t nmemb,
						FILE * __restrict stream)
{
	__STDIO_STREAM_VALIDATE(stream);
	assert(stream->__filedes >= -1);

	/* Note: If nmbem * size > SIZE_MAX then there is an application
	 * bug since no array can be larger than SIZE_MAX in size. */

	if ((__STDIO_STREAM_IS_NARROW_READING(stream)
		 || !__STDIO_STREAM_TRANS_TO_READ(stream, __FLAG_NARROW))
		&& size && nmemb
		) {

		if (nmemb <= (SIZE_MAX / size)) {
			unsigned char *buffer = (unsigned char *) ptr;
			size_t todo, bytes, avail;

			todo = bytes = size * nmemb;

			/* Check for ungots... */
			while (stream->__modeflags & __FLAG_UNGOT) {
				*buffer++ = stream->__ungot[(stream->__modeflags--) & 1];
				stream->__ungot[1] = 0;
				if (!--todo) {
					goto DONE;
				}
			}

#ifdef __STDIO_BUFFERS
			/* Next check for available buffered... */
			if ((avail = stream->__bufread - stream->__bufpos) > 0) {
				if (avail > todo) {
					avail = todo;
				}
				memcpy(buffer, stream->__bufpos, avail);
				buffer += avail;
				stream->__bufpos += avail;
				if (!(todo -= avail)) {
					goto DONE;
				}
			}
 
			/* We need to read from the host environment, so we must
			 * flush all line buffered streams if the stream is not
			 * fully buffered. */
			if (!__STDIO_STREAM_IS_FBF(stream)) {
				__STDIO_FLUSH_LBF_STREAMS;
			}
#endif

#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: should we refill and read from the buffer sometimes?
#endif
			while ((avail = __stdio_READ(stream, buffer, todo)) > 0) {
				buffer += avail;
				if (!(todo -= avail)) {
					break;
				}
			}

		DONE:
			__STDIO_STREAM_VALIDATE(stream);
			return (bytes - todo) / size;
		}

		__STDIO_STREAM_SET_ERROR(stream);
		__set_errno(EINVAL);
	}

	__STDIO_STREAM_VALIDATE(stream);
	return 0;
}

#elif defined __UCLIBC_HAS_THREADS__

size_t fread(void * __restrict ptr, size_t size, size_t nmemb,
			 register FILE * __restrict stream)
{
	size_t retval;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	retval = __fread_unlocked(ptr, size, nmemb, stream);

	__STDIO_AUTO_THREADUNLOCK(stream);

	return retval;
}

#endif
