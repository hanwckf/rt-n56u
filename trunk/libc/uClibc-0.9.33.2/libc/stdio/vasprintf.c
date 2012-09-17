/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include <features.h>

#ifdef __USE_GNU
#include "_stdio.h"
#include <stdarg.h>
#include <bits/uClibc_va_copy.h>


#ifdef __UCLIBC_MJN3_ONLY__
/* Do the memstream stuff inline to avoid fclose and the openlist? */
#warning CONSIDER: avoid open_memstream call?
#endif

#ifndef __STDIO_HAS_VSNPRINTF
#warning Skipping vasprintf since no vsnprintf!
#else

int vasprintf(char **__restrict buf, const char * __restrict format,
			 va_list arg)
{
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__

	FILE *f;
	size_t size;
	int rv = -1;

	*buf = NULL;

	if ((f = open_memstream(buf, &size)) != NULL) {
		rv = vfprintf(f, format, arg);
		fclose(f);
		if (rv < 0) {
			free(*buf);
			*buf = NULL;
		}
	}

	assert(rv >= -1);

	return rv;

#else  /* __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__ */

	/* This implementation actually calls the printf machinery twice, but
	 * only does one malloc.  This can be a problem though when custom printf
	 * specs or the %m specifier are involved because the results of the
	 * second call might be different from the first. */
	va_list arg2;
	int rv;

	va_copy(arg2, arg);
 	rv = vsnprintf(NULL, 0, format, arg2);
	va_end(arg2);

	*buf = NULL;

	if (rv >= 0) {
		if ((*buf = malloc(++rv)) != NULL) {
			if ((rv = vsnprintf(*buf, rv, format, arg)) < 0) {
				free(*buf);
				*buf = NULL;
			}
		}
	}

	assert(rv >= -1);

	return rv;

#endif /* __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__ */
}
libc_hidden_def(vasprintf)

#endif
#endif
