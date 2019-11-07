/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <netdb.h>

#ifndef __UCLIBC_HAS_TLS__
# undef h_errno
extern int h_errno;
#endif

int weak_const_function *__h_errno_location(void)
{
    return &h_errno;
}
