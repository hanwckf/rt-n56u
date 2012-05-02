/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <stdio_ext.h>

/* Solaris function --
 * Discard all buffered data whether reading or writing.
 */

void __fpurge(register FILE * __restrict stream)
{
	__STDIO_STREAM_VALIDATE(stream);

	__STDIO_STREAM_DISABLE_GETC(stream);
	__STDIO_STREAM_DISABLE_PUTC(stream);
	__STDIO_STREAM_INIT_BUFREAD_BUFPOS(stream);
	stream->__ungot[1] = 0;

#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(stream->__state));
#endif
#ifdef __UCLIBC_HAS_WCHAR__
	stream->__ungot_width[0] = 0;
#endif

	stream->__modeflags &= ~(__MASK_READING|__FLAG_WRITING);

	__STDIO_STREAM_VALIDATE(stream);
}
