/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/socket.h>
#include <cancel.h>

#ifndef __NR_accept
#error Missing definition of NR_accept needed for cancellation.
#endif

int
accept (int fd, __SOCKADDR_ARG addr, socklen_t *len)
{
  return _syscall3(int, __NC(accept), int, fd, __SOCKADDR_ARG,
			addr.__sockaddr__, socklen_t*, len);
}

CANCELLABLE_SYSCALL(int, accept, (int fd, __SOCKADDR_ARG addr,
			socklen_t *len), (fd, addr, len))

lt_libc_hidden(accept)
