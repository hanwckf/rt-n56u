/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

ssize_t getline(char **__restrict lineptr, size_t *__restrict n,
				FILE *__restrict stream)
{
	return __getdelim(lineptr, n, '\n', stream);
}
