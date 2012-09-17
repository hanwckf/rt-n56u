/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <stdio_ext.h>

/* Solaris function --
 * Return nonzero if write-only or was last written to, and 0 otherwise.
 */

int __fwriting(FILE * __restrict stream)
{
	__STDIO_STREAM_VALIDATE(stream);

	return __STDIO_STREAM_IS_WRITING_OR_WRITEONLY(stream);
}
