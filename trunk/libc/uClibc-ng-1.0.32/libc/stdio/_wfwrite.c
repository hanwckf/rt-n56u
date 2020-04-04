/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <wchar.h>

#ifndef __UCLIBC_HAS_WCHAR__
#error wide function when no wide support!
#endif

size_t attribute_hidden _wstdio_fwrite(const wchar_t *__restrict ws, size_t n,
					  register FILE *__restrict stream)
{
	size_t r, count;
	char buf[64];
	const wchar_t *pw;

	__STDIO_STREAM_VALIDATE(stream);

#ifdef __STDIO_BUFFERS
	if (__STDIO_STREAM_IS_FAKE_VSWPRINTF(stream)) {
		/* We know buffer is wchar aligned for fake streams. */
		count = (((wchar_t *)(stream->__bufend))
				 - ((wchar_t *)(stream->__bufpos)));
		if (count > n) {
			count = n;
		}
		if (count) {
			wmemcpy((wchar_t *)(stream->__bufpos), ws, count);
			stream->__bufpos = (unsigned char *)(((wchar_t *)(stream->__bufpos)) + count);
		}
		__STDIO_STREAM_VALIDATE(stream);
		return n;
	}
#endif

	count = 0;

	if (__STDIO_STREAM_IS_WIDE_WRITING(stream)
		|| !__STDIO_STREAM_TRANS_TO_WRITE(stream, __FLAG_WIDE)
		) {

		pw = ws;
		while (n > count) {
			r = wcsnrtombs(buf, &pw, n-count, sizeof(buf), &stream->__state);
			if (r != ((size_t) -1)) { /* No encoding errors */
				if (!r) {
					++r;		  /* 0 is returned when nul is reached. */
					pw = ws + count + r; /* pw was set to NULL, so correct. */
				}
				if (__stdio_fwrite((const unsigned char *)buf, r, stream) == r) {
					count = pw - ws;
					continue;
				}
			}
			break;
		}

		/* Note: The count is incorrect if 0 < __stdio_fwrite return < r!!! */
	}

	__STDIO_STREAM_VALIDATE(stream);
	return count;
}
