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

	f.__filedes = filedes;
	f.__modeflags = (__FLAG_NARROW|__FLAG_WRITEONLY|__FLAG_WRITING);

#ifdef __UCLIBC_HAS_WCHAR__
	f.__ungot_width[0] = 0;
#endif
#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.__state));
#endif

/* _vfprintf_internal doesn't do any locking, locking init is here
 * only because of fflush_unlocked. TODO? */
#if (defined(__STDIO_BUFFERS) || defined(__USE_OLD_VFPRINTF__)) && defined(__UCLIBC_HAS_THREADS__)
	f.__user_locking = 1;		/* Set user locking. */
	STDIO_INIT_MUTEX(f.__lock);
#endif
	f.__nextopen = NULL;

#ifdef __USE_OLD_VFPRINTF__
	rv = vfprintf(&f, format, arg);
#else
	rv = _vfprintf_internal(&f, format, arg);
#endif

#ifdef __STDIO_BUFFERS
	/* If not buffering, then fflush is unnecessary. */
	if ((rv > 0) && fflush_unlocked(&f)) {
		rv = -1;
	}
#endif

	assert(rv >= -1);

	return rv;
}
libc_hidden_def(vdprintf)
#endif
