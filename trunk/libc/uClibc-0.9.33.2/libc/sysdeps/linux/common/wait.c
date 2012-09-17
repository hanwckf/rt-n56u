/*
 * Copyright (C) 2006 Steven J. Hill <sjhill@realitydiluted.com>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
#include <stdlib.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

extern __typeof(wait) __libc_wait;
/* Wait for a child to die.  When one does, put its status in *STAT_LOC
 * and return its process ID.  For errors, return (pid_t) -1.  */
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <errno.h>
#include <sysdep-cancel.h>

pid_t attribute_hidden
__libc_wait (__WAIT_STATUS_DEFN stat_loc)
{
  if (SINGLE_THREAD_P)
    return INLINE_SYSCALL (wait4, 4, WAIT_ANY, stat_loc, 0,
			   (struct rusage *) NULL);

  int oldtype = LIBC_CANCEL_ASYNC ();

  pid_t result = INLINE_SYSCALL (wait4, 4, WAIT_ANY, stat_loc, 0,
				 (struct rusage *) NULL);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
#else
/* Wait for a child to die.  When one does, put its status in *STAT_LOC
 * and return its process ID.  For errors, return (pid_t) -1.  */
__pid_t __libc_wait (__WAIT_STATUS_DEFN stat_loc)
{
      return wait4 (WAIT_ANY, stat_loc, 0, (struct rusage *) NULL);
}
#endif
weak_alias(__libc_wait,wait)
