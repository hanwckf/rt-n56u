/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <string.h>


char *__glibc_strerror_r(int errnum, char *strerrbuf, size_t buflen)
{
    __xpg_strerror_r(errnum, strerrbuf, buflen);

    return strerrbuf;
}
libc_hidden_def(__glibc_strerror_r)
