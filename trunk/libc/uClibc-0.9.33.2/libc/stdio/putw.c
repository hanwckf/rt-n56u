/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


/* SUSv2 Legacy function -- need not be reentrant. */

int putw(int w, FILE *stream)
{
#define PW    &w
	/* If w is passed in a register, enable the following. */
#if 0
#undef PW
	int PW[1];
	PW[0] = w;
#endif

#if EOF == -1
	return fwrite_unlocked((void *) PW, sizeof(int), 1, stream) - 1;
#else
	return (fwrite_unlocked((void *) PW, sizeof(int), 1, stream) != 0)
		? 0 : EOF;
#endif
}
