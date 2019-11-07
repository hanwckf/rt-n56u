/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#if SEEK_SET != 0 || SEEK_CUR != 1 || SEEK_END != 2
# error Assumption violated -- values of SEEK_SET, SEEK_CUR, SEEK_END
#endif

#ifndef __DO_LARGEFILE
# define FSEEK         fseek
# define OFFSET_TYPE   long int
#endif

int FSEEK(register FILE *stream, OFFSET_TYPE offset, int whence)
{
#if !defined(__DO_LARGEFILE)

	return fseeko64(stream, offset, whence);

#else

	__offmax_t pos = offset;
	int retval = -1;
	__STDIO_AUTO_THREADLOCK_VAR;

	if (((unsigned int) whence) > 2) {
		__set_errno(EINVAL);
	} else {
		__STDIO_AUTO_THREADLOCK(stream);

		__STDIO_STREAM_VALIDATE(stream);

		if ((!__STDIO_STREAM_IS_WRITING(stream)
			  || !__STDIO_COMMIT_WRITE_BUFFER(stream))
			&& ((whence != SEEK_CUR)
				 || (__stdio_adjust_position(stream, &pos) >= 0))
			&& (__SEEK(stream, &pos, whence) >= 0)
			) {

			/* Clear reading/writing modes, EOF, and ungots. */
			stream->__modeflags &=
				~(__MASK_READING|__FLAG_WRITING|__FLAG_EOF);

			/* Make sure all pointers are reset. */
			__STDIO_STREAM_INIT_BUFREAD_BUFPOS(stream);
			__STDIO_STREAM_DISABLE_GETC(stream);
			__STDIO_STREAM_DISABLE_PUTC(stream);

			/* We reinitialize the mbstate object.  Doing so is
			 * implementation defined behavior. */
#ifdef __STDIO_MBSTATE
			__INIT_MBSTATE(&(stream->__state));
#endif
#ifdef __UCLIBC_HAS_WCHAR__
			stream->__ungot_width[0] = 0;
#endif

			retval = 0;
		}

		__STDIO_STREAM_VALIDATE(stream);

		__STDIO_AUTO_THREADUNLOCK(stream);
	}

	return retval;

#endif
}

#ifdef __DO_LARGEFILE
libc_hidden_def(fseeko64)
#else
libc_hidden_def(fseek)
strong_alias_untyped(fseek,fseeko)
#endif
