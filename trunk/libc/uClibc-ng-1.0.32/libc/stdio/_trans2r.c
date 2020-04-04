/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/* Function to handle transition to reading.
 *   Initialize or verify the stream's orientation (even if writeonly).
 *   Check that the stream is readable.
 *   If currently reading, check that we can transition to writing.
 *      C99 requires that we not be reading, but attempting to
 *      auto-transition by commiting the write buffer is a configurable
 *      option.
 *   Returns 0 on success and EOF otherwise.
 *
 * Notes:
 *   There are two function signatures, depending on wchar support,
 *   since with no wchar support the orientation is narrow by default.
 */

#ifdef __UCLIBC_HAS_WCHAR__
int attribute_hidden __stdio_trans2r_o(FILE * __restrict stream, int oflag)
#else
int attribute_hidden __stdio_trans2r(FILE * __restrict stream)
#endif
{
	__STDIO_STREAM_VALIDATE(stream);
	assert(!__STDIO_STREAM_IS_READING(stream));

#ifdef __UCLIBC_HAS_WCHAR__
	if (!(stream->__modeflags & oflag)) {
		if (stream->__modeflags & (__FLAG_NARROW|__FLAG_WIDE)) {
			__UNDEFINED_OR_NONPORTABLE;
			goto DO_EBADF;
		}
		stream->__modeflags |= oflag;
	}
#endif

	if (stream->__modeflags & __FLAG_WRITEONLY) {
#if defined(__UCLIBC_HAS_WCHAR__) || !defined(__UCLIBC_HAS_STDIO_AUTO_RW_TRANSITION__)
	DO_EBADF:
#endif
		__set_errno(EBADF);
#ifdef __UCLIBC_HAS_STDIO_AUTO_RW_TRANSITION__
	ERROR:
#endif
		__STDIO_STREAM_SET_ERROR(stream);
		__STDIO_STREAM_VALIDATE(stream);
		return EOF;
	}

	if (__STDIO_STREAM_IS_WRITING(stream)) {
#ifdef __UCLIBC_HAS_STDIO_AUTO_RW_TRANSITION__
		if (__STDIO_COMMIT_WRITE_BUFFER(stream)) { /* commit failed! */
			goto ERROR;
		}
		assert(!__STDIO_STREAM_BUFFER_WUSED(stream));

		__STDIO_STREAM_DISABLE_PUTC(stream);
		__STDIO_STREAM_CLEAR_WRITING(stream);
#else
		/* C99: Output shall not be directly followed by input without an
		   intervening call to the fflush function or to a file positioning
		   function (fseek, fsetpos, or rewind). */
		__UNDEFINED_OR_NONPORTABLE;
		goto DO_EBADF;
#endif
	}

	__STDIO_STREAM_SET_READING(stream);
	/* getc macro is enabled when data is read into buffer. */

	__STDIO_STREAM_VALIDATE(stream);
	return 0;
}
