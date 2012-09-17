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

# define fsetpos	fsetpos64
# define fpos_t		fpos64_t
# define fseek		fseeko64
#endif

int fsetpos(FILE *stream, register const fpos_t *pos)
{
#ifdef __STDIO_MBSTATE

	int retval = -1;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	if ((retval = fseek(stream, pos->__pos, SEEK_SET)) == 0) {
		__COPY_MBSTATE(&(stream->__state), &(pos->__mbstate));
		stream->__ungot_width[0]= pos->__mblen_pending;
	}

	__STDIO_AUTO_THREADUNLOCK(stream);

	return retval;

#else

	return fseek(stream, pos->__pos, SEEK_SET);

#endif
}

