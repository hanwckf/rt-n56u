/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/* Function to handle transition to writing.
 *   Initialize or verify the stream's orientation (even if readonly).
 *   Check that the stream is writable.
 *   If currently reading, check that we can transition to writing.
 *      C99 requires that the stream is at EOF, but attempting to
 *      auto-transition via fseek() is a configurable option.
 *   Returns 0 on success and EOF otherwise.
 *
 * Notes:
 *   There are two function signatures, depending on wchar support,
 *   since with no wchar support the orientation is narrow by default.
 */

#ifdef __UCLIBC_HAS_WCHAR__
int __stdio_trans2w_o(FILE * __restrict stream, int oflag)
#else
int __stdio_trans2w(FILE * __restrict stream)
#endif
{
	__STDIO_STREAM_VALIDATE(stream);
	assert(!__STDIO_STREAM_IS_WRITING(stream));

#ifdef __UCLIBC_HAS_WCHAR__
	if (!(stream->__modeflags & oflag)) {
		if (stream->__modeflags & (__FLAG_NARROW|__FLAG_WIDE)) {
			__UNDEFINED_OR_NONPORTABLE;
			goto DO_EBADF;
		}
		stream->__modeflags |= oflag;
	}
#endif

	if (stream->__modeflags & __FLAG_READONLY) {
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

	if (__STDIO_STREAM_IS_READING(stream)) {
		if (!__FEOF_UNLOCKED(stream)) {
#ifdef __UCLIBC_HAS_STDIO_AUTO_RW_TRANSITION__
			/* Need to seek to correct position if we have buffered
			 * read data or ungots.  If appending, we might as well
			 * seek to the end.
			 *
			 * NOTE: If the OS does not handle append files correctly,
			 *   this is insufficient since we would need to seek to
			 *   the end even if not reading.*/
			if (((__STDIO_STREAM_BUFFER_RAVAIL(stream))
				 || (stream->__modeflags & __FLAG_UNGOT))
				&& fseek(stream, 0L,
						 ((stream->__modeflags & __FLAG_APPEND)
						  ? SEEK_END : SEEK_CUR))
				) {
				/* fseek() only sets error indicator on read/write error. */
				goto ERROR;
			}
#else
			/* C99 requires either at EOF or currently not reading. */
			__UNDEFINED_OR_NONPORTABLE;
			goto DO_EBADF;
#endif
		}
		__STDIO_STREAM_CLEAR_READING_AND_UNGOTS(stream);
		__STDIO_STREAM_DISABLE_GETC(stream);
		/* Reaching EOF does not reset buffer pointers... */
		__STDIO_STREAM_INIT_BUFREAD_BUFPOS(stream);
	}

	__STDIO_STREAM_SET_WRITING(stream);
	if (__STDIO_STREAM_IS_NARROW_FBF(stream)) {
		__STDIO_STREAM_ENABLE_PUTC(stream);
	}

	__STDIO_STREAM_VALIDATE(stream);
	return 0;
}
