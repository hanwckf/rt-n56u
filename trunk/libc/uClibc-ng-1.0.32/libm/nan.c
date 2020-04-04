/*
 * Copyright (C) 2002 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/***********************************************************************
    nan, nanf, nanl - return quiet NaN

	These functions shall return a quiet NaN, if available, with content
	indicated through tagp.

	If the implementation does not support quiet NaNs, these functions
	shall return zero.

   Calls:  strlen(), sprintf(), strtod()

***********************************************************************/
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

double nan (const char *tagp)
{
	if (tagp[0] != '\0') {
		char buf[6 + strlen (tagp)];
		sprintf (buf, "NAN(%s)", tagp);
		return strtod (buf, NULL);
	}
	return NAN;
}
libm_hidden_def(nan)

libm_hidden_proto(nanf)
float nanf (const char *tagp)
{
	if (tagp[0] != '\0') {
		char buf[6 + strlen (tagp)];
		sprintf (buf, "NAN(%s)", tagp);
		return strtof (buf, NULL);
	}
	return NAN;
}
libm_hidden_def(nanf)

#if defined __UCLIBC_HAS_LONG_DOUBLE_MATH__ && !defined __NO_LONG_DOUBLE_MATH
libm_hidden_proto(nanl)
long double nanl (const char *tagp)
{
	if (tagp[0] != '\0') {
		char buf[6 + strlen (tagp)];
		sprintf (buf, "NAN(%s)", tagp);
		return strtold (buf, NULL);
	}
	return NAN;
}
libm_hidden_def(nanl)
#endif
