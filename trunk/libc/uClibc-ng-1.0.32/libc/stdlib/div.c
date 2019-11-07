/*
 * div for uClibc
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>

div_t div(int numer, int denom)
{
    div_t result;
    result.quot = numer / denom;
    result.rem  = numer - (result.quot * denom);
    return(result);
}

