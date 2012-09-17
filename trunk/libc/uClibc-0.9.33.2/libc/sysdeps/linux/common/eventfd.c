/* vi: set sw=4 ts=4: */
/*
 * eventfd() for uClibc
 *
 * Copyright (C) 2011 Jean-Christian de Rivaz <jc@eclis.ch>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>

/*
 * eventfd()
 */
#if defined __NR_eventfd || defined __NR_eventfd2
int eventfd (int count, int flags)
{
#if defined __NR_eventfd2
  return INLINE_SYSCALL (eventfd2, 2, count, flags);
#elif defined __NR_eventfd
  if (flags != 0) {
     __set_errno (EINVAL);
    return -1;
  }
  return INLINE_SYSCALL (eventfd, 1, count);
#endif
}
#endif
