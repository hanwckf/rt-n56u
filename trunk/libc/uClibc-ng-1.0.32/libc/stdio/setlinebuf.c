/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifdef __USE_BSD


/* A BSD function.  The implementation matches the linux man page,
 * except that we do not bother calling setvbuf if not configured
 * for stream buffering.
 */

void setlinebuf(FILE * __restrict stream)
{
#ifdef __STDIO_BUFFERS
	setvbuf(stream, NULL, _IOLBF, (size_t) 0);
#endif
}
#endif
