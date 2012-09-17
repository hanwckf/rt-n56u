/*  Copyright (C) 2003     Manuel Novoa III
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  The GNU C Library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with the GNU C Library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA.
 */

/*  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!
 *
 *  This code is currently under development.  Also, I plan to port
 *  it to elks which is a 16-bit environment with a fairly limited
 *  compiler.  Therefore, please refrain from modifying this code
 *  and, instead, pass any bug-fixes, etc. to me.  Thanks.  Manuel
 *
 *  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION! */

#ifndef _UINTMAXTOSTR_H
#define _UINTMAXTOSTR_H 1

#ifdef _FEATURES_H
# ifndef __USE_ISOC99
#  error features was included without defining _ISOC99_SOURCE!
# endif
#else
# ifndef _ISOC99_SOURCE
#  define _ISOC99_SOURCE
# endif
#endif

#include <features.h>
#include <limits.h>
#include <stdint.h>

#if INTMAX_MAX <= 2147483647L
#define __UIM_BUFLEN			12 /* 10 digits + 1 nul + 1 sign */
#elif INTMAX_MAX <= 9223372036854775807LL
#define __UIM_BUFLEN			22 /* 20 digits + 1 nul + 1 sign */
#else
#error unknown number of digits for intmax_t!
#endif

#ifdef LLONG_MAX				/* --------------- */
#if LLONG_MAX <= 2147483647L
#define __UIM_BUFLEN_LLONG		12 /* 10 digits + 1 nul + 1 sign */
#elif LLONG_MAX <= 9223372036854775807LL
#define __UIM_BUFLEN_LLONG		22 /* 20 digits + 1 nul + 1 sign */
#else
#error unknown number of digits for long long!
#endif
#endif /* ULLONG_MAX ----------------------------- */

#if LONG_MAX <= 2147483647L
#define __UIM_BUFLEN_LONG		12 /* 10 digits + 1 nul + 1 sign */
#elif LONG_MAX <= 9223372036854775807LL
#define __UIM_BUFLEN_LONG		22 /* 20 digits + 1 nul + 1 sign */
#else
#error unknown number of digits for long!
#endif

#if INT_MAX <= 32767
#define __UIM_BUFLEN_INT		7 /* 10 digits + 1 nul + 1 sign */
#elif INT_MAX <= 2147483647L
#define __UIM_BUFLEN_INT		12 /* 10 digits + 1 nul + 1 sign */
#else
#error unknown number of digits for int!
#endif

typedef enum {
	__UIM_DECIMAL = 0,
	__UIM_GROUP = ',',			/* Base 10 locale-dependent grouping. */
	__UIM_LOWER = 'a' - 10,
	__UIM_UPPER = 'A' - 10,
} __UIM_CASE;

/* Convert the int val to a string in base abs(base).  val is treated as
 * an unsigned ??? int type if base > 0, and signed if base < 0.  This
 * is an internal function with _no_ error checking done unless assert()s
 * are enabled.
 *
 * Note: bufend is a pointer to the END of the buffer passed.
 * Call like this:
 *     char buf[SIZE], *p;
 *     p = _xltostr(buf + sizeof(buf) - 1, {unsigned int},  10, __UIM_DECIMAL)
 *     p = _xltostr(buf + sizeof(buf) - 1,          {int}, -10, __UIM_DECIMAL)
 *
 * WARNING: If base > 10, case _must_be_ either __UIM_LOWER or __UIM_UPPER
 *          for lower and upper case alphas respectively.
 * WARNING: If val is really a signed type, make sure base is negative!
 *          Otherwise, you could overflow your buffer.
 */
extern char *_uintmaxtostr(char * __restrict bufend, uintmax_t uval,
					int base, __UIM_CASE alphacase) attribute_hidden;

/* TODO -- make this either a (possibly inline) function? */
#ifndef __BCC__
#define _int10tostr(bufend, intval) \
	_uintmaxtostr((bufend), (intval), -10, __UIM_DECIMAL)
#else  /* bcc doesn't do prototypes, we need to explicitly cast */
#define _int10tostr(bufend, intval) \
	_uintmaxtostr((bufend), (uintmax_t)(intval), -10, __UIM_DECIMAL)
#endif

#define __BUFLEN_INT10TOSTR		__UIM_BUFLEN_INT

#endif /* _UINTMAXTOSTR_H */
