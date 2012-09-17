/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/* Note: This is the application-callable ungetwc.  If wscanf calls this, it
 * should also set stream->__ungot[1] to 0 if this is the only ungot, as well
 * as reset stream->__ungot_width[1] for use by _stdio_adjpos().
 */

wint_t ungetwc(wint_t c, register FILE *stream)
{
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);
	__STDIO_STREAM_VALIDATE(stream); /* debugging only */

	/* Note: Even if c == WEOF, we need to initialize/verify the
	 * stream's orientation and ensure the stream is in reading
	 * mode (if readable and properly oriented). */
	if ((!__STDIO_STREAM_IS_WIDE_READING(stream)
		 && __STDIO_STREAM_TRANS_TO_READ(stream, __FLAG_WIDE))
		|| ((stream->__modeflags & __FLAG_UNGOT)
			&& ((stream->__modeflags & 1) || stream->__ungot[1]))
		|| (c == WEOF)
		) {
		c = WEOF;
	} else {
		/* In the wide case, getc macros should already be disabled. */
		/* 	__STDIO_STREAM_DISABLE_GETC(stream); */

		/* Flag this as a user ungot, as scanf does the necessary fixup. */
		stream->__ungot[1] = 1;
		stream->__ungot[(++stream->__modeflags) & 1] = c;
		/* Note: ungot_width is handled by fgetwc. */

		__STDIO_STREAM_CLEAR_EOF(stream); /* Must clear end-of-file flag. */
	}

	__STDIO_STREAM_VALIDATE(stream);
	__STDIO_AUTO_THREADUNLOCK(stream);

	return c;
}
