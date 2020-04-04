/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <time.h>
#include <cancel.h>

/* Pause execution for a number of nanoseconds.  */
int
nanosleep (const struct timespec *requested_time,
             struct timespec *remaining)
{
  return _syscall2(int, __NC(nanosleep), const struct timespec*,
			requested_time, struct timespec* remaining)
}

CANCELLABLE_SYSCALL(int, nanosleep, (const struct timespec *requested_time,
			struct timespec *remaining), (requested_time, remaining))

lt_libc_hidden(nanosleep)
