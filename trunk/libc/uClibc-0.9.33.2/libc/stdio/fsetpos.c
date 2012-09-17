/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifndef __DO_LARGEFILE
#define FSEEK fseek
#endif

libc_hidden_proto(FSEEK)

int fsetpos(FILE *stream, register const fpos_t *pos)
{
#ifdef __STDIO_MBSTATE

	int retval = -1;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	if ((retval = FSEEK(stream, pos->__pos, SEEK_SET)) == 0) {
		__COPY_MBSTATE(&(stream->__state), &(pos->__mbstate));
		stream->__ungot_width[0]= pos->__mblen_pending;
	}

	__STDIO_AUTO_THREADUNLOCK(stream);

	return retval;

#else

	return FSEEK(stream, pos->__pos, SEEK_SET);

#endif
}
