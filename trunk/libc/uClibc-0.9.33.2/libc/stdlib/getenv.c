/* getenv.c for uClibc
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>


/* IEEE Std 1003.1-2001 says getenv need not be thread safe, so
 * don't bother locking access to __environ */
char *getenv(const char *var)
{
    int len;
    char **ep;

    if (!(ep=__environ))
	return NULL;
    len = strlen(var);
    while(*ep) {
	if (memcmp(var, *ep, len) == 0 && (*ep)[len] == '=') {
	    return *ep + len + 1;
	}
	ep++;
    }
    return NULL;
}
libc_hidden_def(getenv)
