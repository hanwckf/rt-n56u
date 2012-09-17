/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <printf.h>

uintmax_t _load_inttype(int desttype, register const void *src, int uflag) attribute_hidden;
uintmax_t _load_inttype(int desttype, register const void *src, int uflag)
{
	if (uflag >= 0) {			/* unsigned */
#if LONG_MAX != INT_MAX
		if (desttype & (PA_FLAG_LONG|PA_FLAG_LONG_LONG)) {
#ifdef LLONG_MAX
			if (desttype == PA_FLAG_LONG_LONG) {
				return *((unsigned long long int *) src);
			}
#endif
			return *((unsigned long int *) src);
		}
#else  /* LONG_MAX != INT_MAX */
#ifdef LLONG_MAX
		if (desttype & PA_FLAG_LONG_LONG) {
			return *((unsigned long long int *) src);
		}
#endif
#endif /* LONG_MAX != INT_MAX */
		{
			unsigned int x;
			x = *((unsigned int *) src);
			if (desttype == __PA_FLAG_CHAR) x = (unsigned char) x;
#if SHRT_MAX != INT_MAX
			if (desttype == PA_FLAG_SHORT) x = (unsigned short int) x;
#endif
			return x;
		}
	} else {					/* signed */
#if LONG_MAX != INT_MAX
		if (desttype & (PA_FLAG_LONG|PA_FLAG_LONG_LONG)) {
#ifdef LLONG_MAX
			if (desttype == PA_FLAG_LONG_LONG) {
				return *((long long int *) src);
			}
#endif
			return *((long int *) src);
		}
#else  /* LONG_MAX != INT_MAX */
#ifdef LLONG_MAX
		if (desttype & PA_FLAG_LONG_LONG) {
			return *((long long int *) src);
		}
#endif
#endif /* LONG_MAX != INT_MAX */
		{
			int x;
			x = *((int *) src);
			if (desttype == __PA_FLAG_CHAR) x = (signed char) x;
#if SHRT_MAX != INT_MAX
			if (desttype == PA_FLAG_SHORT) x = (short int) x;
#endif
			return x;
		}
	}
}
