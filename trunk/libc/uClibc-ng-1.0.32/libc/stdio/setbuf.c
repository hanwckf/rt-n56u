/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


void setbuf(FILE * __restrict stream, register char * __restrict buf)
{
#ifdef __STDIO_BUFFERS
	setvbuf(stream, buf, ((buf != NULL) ? _IOFBF : _IONBF), BUFSIZ);
#endif
}
