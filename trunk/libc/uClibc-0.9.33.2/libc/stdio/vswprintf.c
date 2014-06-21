/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <stdarg.h>
#include <wchar.h>


/* NB: this file is not used if __USE_OLD_VFPRINTF__ */

#ifndef __STDIO_BUFFERS
#warning Skipping vswprintf since no buffering!
#else  /* __STDIO_BUFFERS */

int vswprintf(wchar_t *__restrict buf, size_t size,
			  const wchar_t * __restrict format, va_list arg)
{
	FILE f;
	int rv;

/* 	__STDIO_STREAM_RESET_GCS(&f); */
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
	f.__cookie = &(f.__filedes);
	f.__gcs.read = NULL;
	f.__gcs.write = NULL;
	f.__gcs.seek = NULL;
	f.__gcs.close = NULL;
#endif

	f.__filedes = __STDIO_STREAM_FAKE_VSWPRINTF_FILEDES;
	f.__modeflags = (__FLAG_WIDE|__FLAG_WRITEONLY|__FLAG_WRITING);

	f.__ungot_width[0] = 0;
#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.__state));
#endif /* __STDIO_MBSTATE */

#ifdef __UCLIBC_HAS_THREADS__
	f.__user_locking = 1;		/* Set user locking. */
	STDIO_INIT_MUTEX(f.__lock);
#endif /* __UCLIBC_HAS_THREADS__ */

	f.__nextopen = NULL;

	if (size > ((SIZE_MAX - (size_t) buf)/sizeof(wchar_t))) {
		size = ((SIZE_MAX - (size_t) buf)/sizeof(wchar_t));
	}

	f.__bufstart = (unsigned char *) buf;
	f.__bufend = (unsigned char *) (buf + size);
	__STDIO_STREAM_INIT_BUFREAD_BUFPOS(&f);
	__STDIO_STREAM_DISABLE_GETC(&f);
	__STDIO_STREAM_DISABLE_PUTC(&f);

	rv = _vfwprintf_internal(&f, format, arg);

	/* NOTE: Return behaviour differs from snprintf... */
	if (f.__bufpos == f.__bufend) {
		rv = -1;
		if (size) {
			f.__bufpos = (unsigned char *) (((wchar_t *) f.__bufpos) - 1);
		}
	}
	if (size) {
		*((wchar_t *) f.__bufpos) = 0;
	}
	return rv;
}
libc_hidden_def(vswprintf)

#endif /* __STDIO_BUFFERS */
