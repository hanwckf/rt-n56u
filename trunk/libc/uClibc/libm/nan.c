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

float nanf (const char *tagp)
{
    if (tagp[0] != '\0') {
	char buf[6 + strlen (tagp)];
	sprintf (buf, "NAN(%s)", tagp);
	return strtof (buf, NULL);
    }
    return NAN;
}

#if 0
long double nanl (const char *tagp)
{
    if (tagp[0] != '\0') {
	char buf[6 + strlen (tagp)];
	sprintf (buf, "NAN(%s)", tagp);
	return strtold (buf, NULL);
    }
    return NAN;
}
#endif
