/* Copyright (C) 2004-2005 Manuel Novoa III    <mjn3@codepoet.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/**********************************************************************/

int attribute_hidden __stdio_seek(FILE *stream, register __offmax_t *pos, int whence)
{
	__offmax_t res;

	res = lseek64(stream->__filedes, *pos, whence);

	return (res >= 0) ? ((*pos = res), 0) : ((int) res);
}

/**********************************************************************/
