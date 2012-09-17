/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


FILE *fdopen(int filedes, const char *mode)
{
	intptr_t cur_mode;

	cur_mode = fcntl(filedes, F_GETFL);
	if (cur_mode != -1)
		return _stdio_fopen(cur_mode, mode, NULL, filedes);
	return NULL;
}
libc_hidden_def(fdopen)
