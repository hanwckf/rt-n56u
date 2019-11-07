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
#include <cancel.h>

/* We should deal with kernel which have a smaller UIO_FASTIOV as well
   as a very big count.  */
static ssize_t __NC(readv)(int fd, const struct iovec *vector, int count)
{
	ssize_t bytes_read = INLINE_SYSCALL(readv, 3, fd, vector, count);

	if (bytes_read >= 0 || errno != EINVAL || count <= UIO_FASTIOV)
		return bytes_read;

	/* glibc tries again, but we do not. */
	/* return __atomic_readv_replacement (fd, vector, count); */

	return -1;
}
CANCELLABLE_SYSCALL(ssize_t, readv, (int fd, const struct iovec *vector, int count),
		    (fd, vector, count))
