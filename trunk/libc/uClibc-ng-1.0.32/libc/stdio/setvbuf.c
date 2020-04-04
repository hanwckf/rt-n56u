/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#if (_IOFBF != 0) || (_IOLBF != 1) || (_IONBF != 2)
#error Assumption violated -- values of _IOFBF, _IOLBF, _IONBF
#endif
#if (__FLAG_FBF != 0) || (__FLAG_NBF != (2*__FLAG_LBF))
#error Assumption violated for buffering mode flags
#endif

int setvbuf(register FILE * __restrict stream, register char * __restrict buf,
			int mode, size_t size)
{
#ifdef __STDIO_BUFFERS

	int retval = EOF;
	int alloc_flag = 0;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);
	__STDIO_STREAM_VALIDATE(stream);

	if (((unsigned int) mode) > 2) {
		__set_errno(EINVAL);
		goto ERROR;
	}

	/* C99 states that setvbuf may only be used between a successful
	 * open of the stream and before any other operation other than
	 * an unsuccessful call to setvbuf. */

#ifdef __STDIO_FLEXIBLE_SETVBUF
	/* If we aren't currently reading (including ungots) or writing,
	 * then allow the request to proceed. */

	if (stream->__modeflags & (__MASK_READING|__FLAG_WRITING)) {
		goto ERROR;
	}
#else
	/* The following test isn't quite as strict as C99, as it will
	 * not detect file positioning operations. */

	if (stream->__modeflags & (__MASK_READING|__FLAG_WRITING
							 |__FLAG_NARROW|__FLAG_WIDE
							 |__FLAG_ERROR|__FLAG_EOF)
		) {
		goto ERROR;
	}
#endif

	stream->__modeflags &= ~(__MASK_BUFMODE);	/* Clear current mode */
	stream->__modeflags |= mode * __FLAG_LBF;	/*   and set new one. */

	if ((mode == _IONBF) || !size) {
		size = 0;
		buf = NULL;
	} else if (!buf) {
		if ((__STDIO_STREAM_BUFFER_SIZE(stream) == size) /* Same size or */
			|| !(buf = malloc(size)) /* malloc failed, so don't change. */
			) {
			goto DONE;
		}
		alloc_flag = __FLAG_FREEBUF;
	}

	if (stream->__modeflags & __FLAG_FREEBUF) {
		stream->__modeflags &= ~(__FLAG_FREEBUF);
		free(stream->__bufstart);
	}

	stream->__modeflags |= alloc_flag;
	stream->__bufstart = (unsigned char *) buf;
	stream->__bufend = (unsigned char *) buf + size;
	__STDIO_STREAM_INIT_BUFREAD_BUFPOS(stream);
	__STDIO_STREAM_DISABLE_GETC(stream);
	__STDIO_STREAM_DISABLE_PUTC(stream);

 DONE:
	retval = 0;

 ERROR:
	__STDIO_STREAM_VALIDATE(stream);
	__STDIO_AUTO_THREADUNLOCK(stream);

	return retval;

#else  /* __STDIO_BUFFERS  */

	if (mode == _IONBF) {
		return 0;
	}

	if (((unsigned int) mode) > 2) {
		__set_errno(EINVAL);
	}

	return EOF;

#endif
}
libc_hidden_def(setvbuf)
