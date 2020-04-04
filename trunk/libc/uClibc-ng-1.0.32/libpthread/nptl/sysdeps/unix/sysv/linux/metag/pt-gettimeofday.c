/*
 * Copyright (C) 2013 Imagination Technologies Ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>

int gettimeofday (struct timeval *, struct timezone *) attribute_hidden;
_syscall2(int, gettimeofday, struct timeval *, tv, struct timezone *, tz);
