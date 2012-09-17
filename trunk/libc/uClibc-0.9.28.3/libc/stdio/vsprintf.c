/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <stdarg.h>

#ifndef __STDIO_HAS_VSNPRINTF
#warning Skipping vsprintf since no vsnprintf!
#else

int vsprintf(char *__restrict buf, const char * __restrict format,
			 va_list arg)
{
	return vsnprintf(buf, SIZE_MAX, format, arg);
}

#endif
