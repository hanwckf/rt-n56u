/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifdef __DO_LARGEFILE
# ifndef __UCLIBC_HAS_LFS__
#  error large file support is not enabled!
# endif

# define FTELL				__ftello64
# define OFFSET_TYPE		__off64_t

weak_alias(__ftello64,ftello64);

#else

# define FTELL				ftell
# define OFFSET_TYPE		long int

weak_alias(ftell,ftello);

#endif

OFFSET_TYPE FTELL(register FILE *stream)
{
#if defined(__UCLIBC_HAS_LFS__) && !defined(__DO_LARGEFILE)

	__offmax_t pos = __ftello64(stream);

	if ((sizeof(long) >= sizeof(__offmax_t)) || (((long) pos) == pos)) {
		return ((long) pos);
	} else {
		__set_errno(EOVERFLOW);
		return -1;
	}

#else

	__offmax_t pos = 0;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	__STDIO_STREAM_VALIDATE(stream);

	if ((__SEEK(stream, &pos,
				((__STDIO_STREAM_IS_WRITING(stream)
				  && (stream->__modeflags & __FLAG_APPEND))
				 ? SEEK_END : SEEK_CUR)) < 0)
		|| (__stdio_adjust_position(stream, &pos) < 0)) {
		pos = -1;
	}

	__STDIO_AUTO_THREADUNLOCK(stream);

	return pos;

#endif
}
