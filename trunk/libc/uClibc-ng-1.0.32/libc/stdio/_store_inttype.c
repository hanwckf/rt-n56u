/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <printf.h>

/* Right now, we assume intmax_t is either long or long long */

#ifdef INTMAX_MAX

#ifdef LLONG_MAX

#if INTMAX_MAX > LLONG_MAX
#error INTMAX_MAX > LLONG_MAX!  The printf code needs to be updated!
#endif

#elif INTMAX_MAX > LONG_MAX

#error No LLONG_MAX and INTMAX_MAX > LONG_MAX!  The printf code needs to be updated!

#endif /* LLONG_MAX */

#endif /* INTMAX_MAX */

/* We assume int may be short or long, but short and long are different. */

void _store_inttype(register void *dest, int desttype, uintmax_t val)
{
	if (desttype == __PA_FLAG_CHAR) { /* assume char not int */
		*((unsigned char *) dest) = val;
		return;
	}
#if defined(LLONG_MAX) && (INT_MAX != LLONG_MAX)
	if (desttype == PA_FLAG_LONG_LONG) {
		*((unsigned long long int *) dest) = val;
		return;
	}
#endif /* LLONG_MAX */
#if SHRT_MAX != INT_MAX
	if (desttype == PA_FLAG_SHORT) {
		*((unsigned short int *) dest) = val;
		return;
	}
#endif /* SHRT_MAX */
#if LONG_MAX != INT_MAX
	if (desttype == PA_FLAG_LONG) {
		*((unsigned long int *) dest) = val;
		return;
	}
#endif /* LONG_MAX */

	*((unsigned int *) dest) = val;
}
