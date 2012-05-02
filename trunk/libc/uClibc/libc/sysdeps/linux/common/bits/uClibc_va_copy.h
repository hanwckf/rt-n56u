/* Copyright (C) 2005       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#ifndef _UCLIBC_VA_COPY_H
#define _UCLIBC_VA_COPY_H 1

#include <stdarg.h>

/* Deal with pre-C99 compilers. */
#ifndef va_copy

#ifdef __va_copy
#define va_copy(A,B)	__va_copy(A,B)
#else
#warning Neither va_copy (C99/SUSv3) or __va_copy is defined.  Using a simple copy instead.  But you should really check that this is appropriate and supply an arch-specific override if necessary.
	/* the glibc manual suggests that this will usually suffice when
        __va_copy doesn't exist.  */
#define va_copy(A,B)	A = B
#endif

#endif /* va_copy */

#endif /* _UCLIBC_VA_COPY_H */
