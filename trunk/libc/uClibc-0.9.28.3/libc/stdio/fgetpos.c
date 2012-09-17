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

# define fgetpos	fgetpos64
# define fpos_t		fpos64_t
# define ftell		ftello64
#endif

int fgetpos(FILE * __restrict stream, register fpos_t * __restrict pos)
{
#ifdef __STDIO_MBSTATE

	int retval = -1;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	if ((pos->__pos = ftell(stream)) >= 0) {
		__COPY_MBSTATE(&(pos->__mbstate), &(stream->__state));
		pos->__mblen_pending = stream->__ungot_width[0];
		retval = 0;
	}

	__STDIO_AUTO_THREADUNLOCK(stream);

	return retval;

#else

	return ((pos->__pos = ftell(stream)) >= 0) ? 0 : -1;

#endif
}
