/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"



#ifdef __DO_UNLOCKED

static void munge_stream(register FILE *stream, unsigned char *buf)
{
	stream->__bufend = stream->__bufstart = buf;
	__STDIO_STREAM_INIT_BUFREAD_BUFPOS(stream);
	__STDIO_STREAM_DISABLE_GETC(stream);
	__STDIO_STREAM_DISABLE_PUTC(stream);
}

wint_t fgetwc_unlocked(register FILE *stream)
{
	wint_t wi;
	wchar_t wc[1];
	int n;
	size_t r;
	unsigned char sbuf[1];

	__STDIO_STREAM_VALIDATE(stream);

	wi = WEOF;					/* Prepare for failure. */

	if (__STDIO_STREAM_IS_WIDE_READING(stream)
		|| !__STDIO_STREAM_TRANS_TO_READ(stream, __FLAG_WIDE)
		) {
		if (stream->__modeflags & __FLAG_UNGOT) { /* Any ungetwc()s? */
			if (((stream->__modeflags & 1) || stream->__ungot[1])) {
				stream->__ungot_width[0] = 0;	/* Application ungot... */
			} else {			/* scanf ungot */
				stream->__ungot_width[0] = stream->__ungot_width[1];
			}

			wi = stream->__ungot[(stream->__modeflags--) & 1];
			stream->__ungot[1] = 0;
			goto DONE;
		}

		if (!stream->__bufstart) {	/* Ugh... stream isn't buffered! */
			/* Munge the stream temporarily to use a 1-byte buffer. */
			munge_stream(stream, sbuf);
			++stream->__bufend;
		}

		if (stream->__state.__mask == 0) { /* If last was a complete char */
			stream->__ungot_width[0] = 0; /* then reset the width. */
		}

 LOOP:
		if ((n = __STDIO_STREAM_BUFFER_RAVAIL(stream)) == 0) {
			goto FILL_BUFFER;
		}

		r = mbrtowc(wc, (const char*) stream->__bufpos, n, &stream->__state);
		if (((ssize_t) r) >= 0) { /* Success... */
			if (r == 0) { /* Nul wide char... means 0 byte for us so */
				++r;	 /* increment r and handle below as single. */
			}
			stream->__bufpos += r;
			stream->__ungot_width[0] += r;
			wi = *wc;
			goto DONE;
		}

		if (r == ((size_t) -2)) {
			/* Potentially valid but incomplete and no more buffered. */
			stream->__bufpos += n; /* Update bufpos for stream. */
			stream->__ungot_width[0] += n;
 FILL_BUFFER:
			if(__STDIO_FILL_READ_BUFFER(stream)) { /* Refill succeeded? */
				goto LOOP;
			}
			if (!__FERROR_UNLOCKED(stream)) { /* EOF with no error. */
				if (!stream->__state.__mask) { /* No partial wchar. */
					goto DONE;
				}
				/* EOF but partially complete wchar. */
				/* TODO: should EILSEQ be set? */
				__set_errno(EILSEQ);
			}
		}

		/* If we reach here, either r == ((size_t)-1) and mbrtowc set errno
		 * to EILSEQ, or r == ((size_t)-2) and stream is in an error state
		 * or at EOF with a partially complete wchar.  Make sure stream's
		 * error indicator is set. */
		stream->__modeflags |= __FLAG_ERROR;

 DONE:
		if (stream->__bufstart == sbuf) { /* Need to un-munge the stream. */
			munge_stream(stream, NULL);
		}

	}

	__STDIO_STREAM_VALIDATE(stream);

	return wi;
}
libc_hidden_def(fgetwc_unlocked)

strong_alias(fgetwc_unlocked,getwc_unlocked)
#ifndef __UCLIBC_HAS_THREADS__
strong_alias(fgetwc_unlocked,fgetwc)
libc_hidden_def(fgetwc)

strong_alias(fgetwc_unlocked,getwc)
#endif

#elif defined __UCLIBC_HAS_THREADS__

wint_t fgetwc(register FILE *stream)
{
	wint_t retval;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	retval = fgetwc_unlocked(stream);

	__STDIO_AUTO_THREADUNLOCK(stream);

	return retval;
}
libc_hidden_def(fgetwc)

strong_alias(fgetwc,getwc)
#endif
