/*
 * writev() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/uio.h>
#include <cancel.h>

/* We should deal with kernel which have a smaller UIO_FASTIOV as well
   as a very big count.  */
static ssize_t __NC(writev)(int fd, const struct iovec *vector, int count)
{
	ssize_t	bytes_written = INLINE_SYSCALL(writev, 3, fd, vector, count);

	if (bytes_written >= 0 || errno != EINVAL || count <= UIO_FASTIOV)
		return bytes_written;

	/* glibc tries again, but we do not. */
	/* return __atomic_writev_replacement (fd, vector, count); */

	return -1;
}
CANCELLABLE_SYSCALL(ssize_t, writev, (int fd, const struct iovec *vector, int count), (fd, vector, count))
