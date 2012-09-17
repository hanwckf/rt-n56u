/* vi: set sw=4 ts=4: */
/*
 * sched_getcpu() for uClibc
 *
 * Copyright (C) 2011 Bernhard Reutner-Fischer <uclibc@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <sysdep.h>

#if defined __NR_getcpu
int
sched_getcpu (void)
{
  unsigned int cpu;
  int r = INLINE_SYSCALL (getcpu, 3, &cpu, NULL, NULL);

  return r == -1 ? r : cpu;
}
#endif
