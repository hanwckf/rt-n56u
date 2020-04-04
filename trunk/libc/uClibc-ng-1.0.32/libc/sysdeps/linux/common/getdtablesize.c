/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>

/* XXX: _BSD || _XOPEN_SOURCE >= 500 */
#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED


#define __LOCAL_OPEN_MAX	    256

/* Return the maximum number of file descriptors
   the current process could possibly have.  */
int getdtablesize (void)
{
  struct rlimit ru;

  /* This should even work if `getrlimit' is not implemented.  POSIX.1
     does not define this function but we will generate a stub which
     returns -1.  */
  return getrlimit (RLIMIT_NOFILE, &ru) < 0 ? __LOCAL_OPEN_MAX : ru.rlim_cur;
}
libc_hidden_def(getdtablesize)
#endif
