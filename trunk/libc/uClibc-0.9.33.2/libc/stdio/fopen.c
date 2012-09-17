/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifndef __DO_LARGEFILE
# define FILEDES_ARG    (-1)
# undef fopen
#else
# undef fopen64
#endif

FILE *fopen(const char * __restrict filename, const char * __restrict mode)
{
	return _stdio_fopen(((intptr_t) filename), mode, NULL, FILEDES_ARG);
}
libc_hidden_def(fopen)
