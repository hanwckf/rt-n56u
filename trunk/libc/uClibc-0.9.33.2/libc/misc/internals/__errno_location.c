/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "internal_errno.h"

/* psm: moved to bits/errno.h: */
int *
#ifndef __UCLIBC_HAS_THREADS__
weak_const_function
#endif
__errno_location (void)
{
    return &errno;
}
#ifdef IS_IN_libc /* not really need, only to keep in sync w/ libc_hidden_proto */
libc_hidden_weak(__errno_location)
#endif
