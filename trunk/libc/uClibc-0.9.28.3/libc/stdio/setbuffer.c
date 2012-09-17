/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/* A BSD function.  The implementation matches the linux man page,
 * except that we do not bother calling setvbuf if not configured
 * for stream buffering.
 */

void setbuffer(FILE * __restrict stream, register char * __restrict buf,
			   size_t size)
{
#ifdef __STDIO_BUFFERS
	setvbuf(stream, buf, (buf ? _IOFBF : _IONBF), size);
#endif
}
