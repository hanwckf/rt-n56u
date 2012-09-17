/* vi: set sw=4 ts=4: */
/*
 * readv() for uClibc
 *
 * Copyright (C) 2006 by Steven J. Hill <sjhill@realitydiluted.com>
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/uio.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <sysdep-cancel.h>

/* We should deal with kernel which have a smaller UIO_FASTIOV as well
   as a very big count.  */
static ssize_t __readv (int fd, const struct iovec *vector, int count)
{
  ssize_t bytes_read;

  bytes_read = INLINE_SYSCALL (readv, 3, fd, vector, count);

  if (bytes_read >= 0 || errno != EINVAL || count <= UIO_FASTIOV)
    return bytes_read;

  /* glibc tries again, but we do not. */
  //return __atomic_readv_replacement (fd, vector, count);

  return -1;
}

ssize_t readv (int fd, const struct iovec *vector, int count)
{
  if (SINGLE_THREAD_P)
    return __readv (fd, vector, count);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = __readv (fd, vector, count);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
#else
_syscall3(ssize_t, readv, int, filedes, const struct iovec *, vector,
		  int, count)
#endif
