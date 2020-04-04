/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include <features.h>

#ifdef __USE_GNU
#include "_stdio.h"

#ifndef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
#error no custom streams!
#endif

/* NOTE: GLIBC difference!!! -- fopencookie
 * According to the info pages, glibc allows seeking within buffers even if
 * no seek function is supplied.  We don't. */

/* NOTE: GLIBC difference!!! -- fopencookie
 * When compiled without large file support, the offset pointer for the
 * cookie_seek function is off_t * and not off64_t * as for glibc. */

/* NOTE: GLIBC difference!!! -- fopencookie (bcc only)
 * Since bcc doesn't support passing of structs, we define fopencookie as a
 * macro in terms of _fopencookie which takes a struct * for the io functions
 * instead.
 */

/* Currently no real reentrancy issues other than a possible double close(). */

#ifndef __BCC__
FILE *fopencookie(void * __restrict cookie, const char * __restrict mode,
				  cookie_io_functions_t io_functions)
#else
FILE *_fopencookie(void * __restrict cookie, const char * __restrict mode,
				   register cookie_io_functions_t *io_functions)
#endif
{
	FILE *stream;
	_IO_cookie_file_t *new_f;

	new_f = malloc(sizeof(_IO_cookie_file_t));
	if (new_f == NULL) {
		return NULL;
	}
	new_f->__fp.__modeflags = __FLAG_FREEFILE;
#ifdef __STDIO_BUFFERS
	new_f->__fp.__bufstart = NULL; /* We allocate a buffer below. */
#endif
#ifdef __UCLIBC_HAS_THREADS__
	/* We only initialize the mutex in the non-freopen case. */
	STDIO_INIT_MUTEX(new_f->__fp.__lock);
#endif
	/* Fake an fdopen guaranteed to pass the _stdio_fopen basic agreement
	 * check without an fcntl call. */
	stream = _stdio_fopen(((intptr_t)(INT_MAX-1)), mode, &new_f->__fp, INT_MAX);
	if (stream) {
		stream->__filedes = __STDIO_STREAM_GLIBC_CUSTOM_FILEDES;
#ifndef __BCC__
		new_f->__gcs = io_functions;
#else
		new_f->__gcs.read  = io_functions->read;
		new_f->__gcs.write = io_functions->write;
		new_f->__gcs.seek  = io_functions->seek;
		new_f->__gcs.close = io_functions->close;
#endif
		new_f->__cookie = cookie;

		__STDIO_STREAM_VALIDATE(stream);
	}

	return stream;
}
#ifndef __BCC__
libc_hidden_def(fopencookie)
#endif
#endif
