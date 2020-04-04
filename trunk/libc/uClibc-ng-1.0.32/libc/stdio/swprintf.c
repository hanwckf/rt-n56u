/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <stdarg.h>
#include <wchar.h>


#ifndef __STDIO_BUFFERS
#warning Skipping swprintf since no buffering!
#else  /* __STDIO_BUFFERS */

int swprintf(wchar_t *__restrict buf, size_t size,
			 const wchar_t * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vswprintf(buf, size, format, arg);
	va_end(arg);

	return rv;
}

#endif /* __STDIO_BUFFERS */
