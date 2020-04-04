/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/socket.h>
#include <cancel.h>

#ifndef __NR_connect
#error Missing definition of NR_connect needed for cancellation.
#endif

int
connect (int fd, __CONST_SOCKADDR_ARG addr, socklen_t len)
{
  return _syscall3(int, __NC(connect), int, fd, __CONST_SOCKADDR_ARG,
			addr.__sockaddr__, socklen_t, len);
}

CANCELLABLE_SYSCALL(int, connect, (int fd, __CONST_SOCKADDR_ARG addr,
			socklen_t len), (fd, addr, len))

lt_libc_hidden(connect)
