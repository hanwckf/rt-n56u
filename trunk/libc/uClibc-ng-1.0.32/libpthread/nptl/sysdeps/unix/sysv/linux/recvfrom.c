/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/socket.h>
#include <cancel.h>

#ifndef __NR_recvfrom
#error Missing definition of NR_recvfrom needed for cancellation.
#endif

ssize_t
recvfrom (int fd, const void *buf, size_t len, int flags,
	       __SOCKADDR_ARG addr, socklen_t addrlen)
{
  return _syscall6(ssize_t, __NC(recvfrom), int, fd, const void* buf,
			size_t, len, int, flags, __SOCKADDR_ARG,
			addr.__sockaddr__, socklen_t, addrlen);
}

CANCELLABLE_SYSCALL(ssize_t, recvfrom, (int fd, const void *buf,
			size_t len, int flags, __SOCKADDR_ARG addr,
			socklen_t addrlen), (fd, buf, len, flags, addr, addrlen))

lt_libc_hidden(recvfrom)
