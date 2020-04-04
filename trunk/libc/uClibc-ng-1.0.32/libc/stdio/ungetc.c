/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/* Having ungotten characters implies the stream is reading.
 * The scheme used here treats the least significant 2 bits of
 * the stream's modeflags member as follows:
 *   0 0   Not currently reading.
 *   0 1   Reading, but no ungetc() or scanf() push back chars.
 *   1 0   Reading with one ungetc() char (ungot[1] is 1)
 *         or one scanf() pushed back char (ungot[1] is 0).
 *   1 1   Reading with both an ungetc() char and a scanf()
 *         pushed back char.  Note that this must be the result
 *         of a scanf() push back (in ungot[0]) _followed_ by
 *         an ungetc() call (in ungot[1]).
 *
 * Notes:
 *   scanf() can NOT use ungetc() to push back characters.
 *     (See section 7.19.6.2 of the C9X rationale -- WG14/N897.)
 */

int ungetc(int c, register FILE *stream)
{
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);
	__STDIO_STREAM_VALIDATE(stream);

#ifdef __UCLIBC_HAS_STDIO_GETC_MACRO__
	/* If buffered narrow reading with no ungot slots filled, and if not
	 * ungetting a different char than the one last read from the buffer,
	 * we can simply decrement the position and not worry about disabling
	 * the getc macros.  This will cut down on overhead in applications
	 * that use getc/ungetc extensively (like gcc). */
	/* NOTE: If we can use getc, then we are buffered narrow reading with
	 * no ungot slots filled. */
	if (__STDIO_STREAM_CAN_USE_BUFFER_GET(stream)
		&& (c != EOF)
		&& (stream->__bufpos > stream->__bufstart)
		&& (stream->__bufpos[-1] == ((unsigned char)c))
		) {
		--stream->__bufpos;
		__STDIO_STREAM_CLEAR_EOF(stream); /* Must clear end-of-file flag. */
	} else
#endif
	/* Note: Even if c == EOF, we need to initialize/verify the
	 * stream's orientation and ensure the stream is in reading
	 * mode (if readable and properly oriented). */
	if ((!__STDIO_STREAM_IS_NARROW_READING(stream)
		 && __STDIO_STREAM_TRANS_TO_READ(stream, __FLAG_NARROW))
		|| ((stream->__modeflags & __FLAG_UNGOT)
			&& ((stream->__modeflags & 1) || stream->__ungot[1]))
		) {
		c = EOF;
	} else if (c != EOF) {
		__STDIO_STREAM_DISABLE_GETC(stream);

		/* Flag this as a user ungot, as scanf does the necessary fixup. */
		stream->__ungot[1] = 1;
		stream->__ungot[(++stream->__modeflags) & 1] = c;

		__STDIO_STREAM_CLEAR_EOF(stream); /* Must clear end-of-file flag. */
	}

	__STDIO_STREAM_VALIDATE(stream);
	__STDIO_AUTO_THREADUNLOCK(stream);

	return c;
}
libc_hidden_def(ungetc)
