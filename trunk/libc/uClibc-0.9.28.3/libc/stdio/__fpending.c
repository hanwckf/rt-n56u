/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <stdio_ext.h>

/* Solaris function --
 * Returns the number of bytes in the buffer for a writing stream.
 *
 * NOTE: GLIBC DIFFERENCE!!!
 *
 * glibc will return the number of wide chars pending for wide oriented
 * streams.  We always return the number of bytes in the buffer, as we
 * convert wide chars to their multibyte encodings and buffer _those_.
 */

#ifdef __UCLIBC_HAS_WCHAR__
#warning Note: Unlike the glibc version, this __fpending returns bytes in buffer for wide streams too!

link_warning(__fpending, "This version of __fpending returns bytes remaining in buffer for both narrow and wide streams.  glibc's version returns wide chars in buffer for the wide stream case.")

#endif

size_t __fpending(register FILE * __restrict stream)
{
	__STDIO_STREAM_VALIDATE(stream);

	return (__STDIO_STREAM_IS_WRITING(stream))
		? __STDIO_STREAM_BUFFER_WUSED(stream)
		: 0;
}
