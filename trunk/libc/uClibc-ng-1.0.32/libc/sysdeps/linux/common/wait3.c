/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
# include <sys/wait.h>

pid_t wait3(__WAIT_STATUS stat_loc, int options, struct rusage *usage)
{
      return __wait4_nocancel(WAIT_ANY, stat_loc, options, usage);
}
#endif
