/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/* Both ftell() and fseek() (for SEEK_CUR) need to correct the stream's
 * position to take into account buffered data and ungotten chars.
 *
 * If successful, store corrected position in *pos and return >= 0.
 * Otherwise return < 0.
 *
 * If position is unrepresentable, set errno to EOVERFLOW.
 */

int __stdio_adjust_position(register FILE * __restrict stream,
							register __offmax_t *pos)
{
	__offmax_t oldpos;
	int corr;

	if ((corr = stream->__modeflags & __MASK_READING) != 0) {
		--corr;	/* Correct for ungots. Assume narrow, and fix below. */
	}

#ifdef __UCLIBC_HAS_WCHAR__
	if (corr && __STDIO_STREAM_IS_WIDE(stream)) {
		/* A wide stream and we have at least one ungotten wchar.
		 * If it is a user ungot, we need to fail since position
		 * is unspecified as per C99. */
		if ((corr > 1) || stream->__ungot[1]) { /* User ungetwc, */
			return -1;			/* so position is undefined. */
		}
		corr -= (1 + stream->__ungot_width[1]);
		if (stream->__state.__mask > 0) { /* Incomplete (bad?) mb char. */
			corr -= stream->__ungot_width[0];
		}
	}
#endif

#ifdef __STDIO_BUFFERS
	corr += (((__STDIO_STREAM_IS_WRITING(stream))
			  ? stream->__bufstart : stream->__bufread)
			 - stream->__bufpos);
#endif

	oldpos = *pos;

	/* Range checking cases:
	 * (pos - corr >  pos) && (corr >  0) : underflow?  return -corr < 0
	 * (pos - corr >  pos) && (corr <  0) : ok .. return -corr > 0
	 * (pos - corr <= pos) && (corr >= 0) : ok .. return  corr > 0
	 * (pos - corr <= pos) && (corr <  0) : overflow ..  return corr < 0
	 */

	if ((*pos -= corr) > oldpos) {
		corr = -corr;
	}

	if (corr < 0) {
		__set_errno(EOVERFLOW);
	}

	return corr;
}
