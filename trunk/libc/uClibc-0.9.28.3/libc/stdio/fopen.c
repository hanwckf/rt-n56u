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

# define fopen			fopen64
# define FILEDES_ARG    (-2)
#else
# define FILEDES_ARG    (-1)
#endif

FILE *fopen(const char * __restrict filename, const char * __restrict mode)
{
	return _stdio_fopen(((intptr_t) filename), mode, NULL, FILEDES_ARG);
}
