/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <stdio_ext.h>

/* Solaris function --
 * Returns the size of the buffer in bytes..
 */

size_t __fbufsize(register FILE * __restrict stream)
{
	__STDIO_STREAM_VALIDATE(stream);

	return __STDIO_STREAM_BUFFER_SIZE(stream);
}
