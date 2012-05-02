/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/* SUSv2 Legacy function -- need not be reentrant. */

int getw(FILE *stream)
{
	int aw;

	return (__fread_unlocked((void *) &aw, sizeof(int), 1, stream) != 0)
		? aw : EOF;
}
