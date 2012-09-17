/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#undef fputc
#undef fputc_unlocked
#undef putc
#undef putc_unlocked


#ifdef __DO_UNLOCKED

int __fputc_unlocked(int c, register FILE *stream)
{
	__STDIO_STREAM_VALIDATE(stream);

	/* First the fast path.  We're good to go if putc macro enabled. */
	if (__STDIO_STREAM_CAN_USE_BUFFER_ADD(stream)) {
		__STDIO_STREAM_BUFFER_ADD(stream, ((unsigned char) c));
		return (unsigned char) c;
	}

	/* Next quickest... writing and narrow oriented, but macro
	 * disabled and/or buffer is full. */
	if (__STDIO_STREAM_IS_NARROW_WRITING(stream)
		|| !__STDIO_STREAM_TRANS_TO_WRITE(stream, __FLAG_NARROW)
		) {
		if (__STDIO_STREAM_IS_FAKE_VSNPRINTF(stream)) {
			return (unsigned char) c;
		}

		if (__STDIO_STREAM_BUFFER_SIZE(stream)) { /* Do we have a buffer? */
			/* The buffer is full and/or the stream is line buffered. */
			if (!__STDIO_STREAM_BUFFER_WAVAIL(stream) /* Buffer full? */
				&& __STDIO_COMMIT_WRITE_BUFFER(stream) /* Commit failed! */
				) {
				goto BAD;
			}
#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: Should we fail if the commit fails but we now have room?
#endif

			__STDIO_STREAM_BUFFER_ADD(stream, ((unsigned char) c));

			if (__STDIO_STREAM_IS_LBF(stream)) {
				if ((((unsigned char) c) == '\n')
					&& __STDIO_COMMIT_WRITE_BUFFER(stream)) {
					/* Commit failed! */
					__STDIO_STREAM_BUFFER_UNADD(stream); /* Undo the write! */
					goto BAD;
				}
			}
		} else {
			/* NOTE: Do not try to save space by moving uc to the top of
			 * the file, as that dramaticly increases runtime. */
			unsigned char uc = (unsigned char) c;
			if (! __stdio_WRITE(stream, &uc, 1)) {
				goto BAD;
			}
		}
		return (unsigned char) c;
	}

 BAD:
	return EOF;
}
libc_hidden_def(__fputc_unlocked)

strong_alias(__fputc_unlocked,fputc_unlocked)
libc_hidden_def(fputc_unlocked)

strong_alias(__fputc_unlocked,putc_unlocked)
libc_hidden_def(putc_unlocked)
#ifndef __UCLIBC_HAS_THREADS__
strong_alias(__fputc_unlocked,fputc)
libc_hidden_def(fputc)

strong_alias(__fputc_unlocked,putc)
libc_hidden_def(putc)
#endif

#elif defined __UCLIBC_HAS_THREADS__

int fputc(int c, register FILE *stream)
{
	if (stream->__user_locking != 0) {
		return __PUTC_UNLOCKED_MACRO(c, stream);
	} else {
		int retval;
		__STDIO_ALWAYS_THREADLOCK(stream);
		retval = __PUTC_UNLOCKED_MACRO(c, stream);
		__STDIO_ALWAYS_THREADUNLOCK(stream);
		return retval;
	}
}
libc_hidden_def(fputc)

strong_alias(fputc,putc)
libc_hidden_def(putc)

#endif
