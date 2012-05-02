/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <stdarg.h>

int vdprintf(int filedes, const char * __restrict format, va_list arg)
{
	FILE f;
	int rv;
#ifdef __STDIO_BUFFERS
	char buf[64];				/* TODO: provide _optional_ buffering? */

	f.__bufend = (unsigned char *) buf + sizeof(buf);
	f.__bufstart = (unsigned char *) buf;
	__STDIO_STREAM_DISABLE_GETC(&f);
	__STDIO_STREAM_DISABLE_PUTC(&f);
	__STDIO_STREAM_INIT_BUFREAD_BUFPOS(&f);
#endif

/* 	__STDIO_STREAM_RESET_GCS(&f); */
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
	f.__cookie = &(f.__filedes);
	f.__gcs.read = NULL;
	f.__gcs.write = _cs_write;
	f.__gcs.seek = NULL;
	f.__gcs.close = NULL;
#endif

	f.__filedes = filedes;
	f.__modeflags = (__FLAG_NARROW|__FLAG_WRITEONLY|__FLAG_WRITING);

#ifdef __UCLIBC_HAS_WCHAR__
	f.__ungot_width[0] = 0;
#endif /* __UCLIBC_HAS_WCHAR__ */
#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.__state));
#endif /* __STDIO_MBSTATE */

#ifdef __UCLIBC_HAS_THREADS__
	f.__user_locking = 1;		/* Set user locking. */
	__stdio_init_mutex(&f.__lock);
#endif
	f.__nextopen = NULL;

	rv = vfprintf(&f, format, arg);

#ifdef __STDIO_BUFFERS
	/* If not buffering, then fflush is unnecessary. */
	if ((rv > 0) && __fflush_unlocked(&f)) {
		rv = -1;
	}
#endif

	assert(rv >= -1);

	return rv;
}
