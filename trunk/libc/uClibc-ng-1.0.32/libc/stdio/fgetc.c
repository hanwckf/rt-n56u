/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#undef fgetc
#undef fgetc_unlocked
#undef getc
#undef getc_unlocked


#ifdef __DO_UNLOCKED


int __fgetc_unlocked(FILE *stream)
{
	__STDIO_STREAM_VALIDATE(stream);

	/* First the fast path.  We're good to go if getc macro enabled. */
	if (__STDIO_STREAM_CAN_USE_BUFFER_GET(stream)) {
		return __STDIO_STREAM_BUFFER_GET(stream);
	}

	/* Next quickest... reading and narrow oriented, but macro
	 * disabled and/or buffer is exhausted. */
	if (__STDIO_STREAM_IS_NARROW_READING(stream)
		|| !__STDIO_STREAM_TRANS_TO_READ(stream, __FLAG_NARROW)
		) {
		if (stream->__modeflags & __FLAG_UNGOT) { /* Use ungots first. */
			unsigned char uc = stream->__ungot[(stream->__modeflags--) & 1];
			stream->__ungot[1] = 0;
			__STDIO_STREAM_VALIDATE(stream);
			return uc;
		}

		if (__STDIO_STREAM_BUFFER_RAVAIL(stream)) {	/* Have buffered? */
			return __STDIO_STREAM_BUFFER_GET(stream);
		}

		/* Is this a fake stream for *sscanf? */
		if (__STDIO_STREAM_IS_FAKE_VSSCANF(stream)) {
			__STDIO_STREAM_SET_EOF(stream);
			return EOF;
		}

		/* We need to read from the host environment, so we must
		 * flush all line buffered streams if the stream is not
		 * fully buffered. */
		if (!__STDIO_STREAM_IS_FBF(stream)) {
			__STDIO_FLUSH_LBF_STREAMS;
		}

		if (__STDIO_STREAM_BUFFER_SIZE(stream)) { /* Do we have a buffer? */
			__STDIO_STREAM_DISABLE_GETC(stream);
			if(__STDIO_FILL_READ_BUFFER(stream)) {	/* Refill succeeded? */
				__STDIO_STREAM_ENABLE_GETC(stream);	/* FBF or LBF */
				return __STDIO_STREAM_BUFFER_GET(stream);
			}
		} else {
			unsigned char uc;
			if (__stdio_READ(stream, &uc, 1)) {
				return uc;
			}
		}
	}

	return EOF;
}
libc_hidden_def(__fgetc_unlocked)

strong_alias(__fgetc_unlocked,fgetc_unlocked)
libc_hidden_def(fgetc_unlocked)

strong_alias(__fgetc_unlocked,getc_unlocked)
libc_hidden_def(getc_unlocked)

#ifndef __UCLIBC_HAS_THREADS__
strong_alias(__fgetc_unlocked,fgetc)
libc_hidden_def(fgetc)

strong_alias(__fgetc_unlocked,getc)
#endif

#elif defined __UCLIBC_HAS_THREADS__

int fgetc(register FILE *stream)
{
	if (stream->__user_locking != 0) {
		return __GETC_UNLOCKED_MACRO(stream);
	} else {
		int retval;
		__STDIO_ALWAYS_THREADLOCK(stream);
		retval = __GETC_UNLOCKED_MACRO(stream);
		__STDIO_ALWAYS_THREADUNLOCK(stream);
		return retval;
	}
}
libc_hidden_def(fgetc)

strong_alias(fgetc,getc)

#endif
