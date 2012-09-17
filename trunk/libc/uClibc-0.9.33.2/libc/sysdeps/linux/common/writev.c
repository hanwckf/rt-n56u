/* vi: set sw=4 ts=4: */
/*
 * writev() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/uio.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <errno.h>
#include <sysdep-cancel.h>

/* We should deal with kernel which have a smaller UIO_FASTIOV as well
   as a very big count.  */
static ssize_t __writev (int fd, const struct iovec *vector, int count)
{
  ssize_t bytes_written;

  bytes_written = INLINE_SYSCALL (writev, 3, fd, vector, count);

  if (bytes_written >= 0 || errno != EINVAL || count <= UIO_FASTIOV)
    return bytes_written;

  /* glibc tries again, but we do not. */
  /* return __atomic_writev_replacement (fd, vector, count); */

  return -1;
}

ssize_t writev (int fd, const struct iovec *vector, int count)
{
  if (SINGLE_THREAD_P)
    return __writev (fd, vector, count);

  int oldtype = LIBC_CANCEL_ASYNC ();

  ssize_t result = __writev (fd, vector, count);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
#else
_syscall3(ssize_t, writev, int, filedes, const struct iovec *, vector,
		  int, count)
#endif
