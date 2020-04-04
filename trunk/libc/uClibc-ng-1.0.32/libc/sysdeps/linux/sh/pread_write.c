/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* SuperH doesn't have this alignment issue.  It just decided to copy
 * the syscall interface from another arch for no good reason. */
#define __UCLIBC_SYSCALL_ALIGN_64BIT__
#include "../common/pread_write.c"
