/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/socket.h>
#include <cancel.h>

#ifndef __NR_recvmsg
#error Missing definition of NR_recvmsg needed for cancellation.
#endif

ssize_t
recvmsg (int fd, const struct msghdr *msg, int flags)
{
  return _syscall3(ssize_t, __NC(recvmsg), int, fd, const struct msghdr*, msg,
			int, flags);
}

CANCELLABLE_SYSCALL(ssize_t, recvmsg, (int fd, const struct msghdr *msg,
			int flags), (fd, msg, flags))

lt_libc_hidden(recvmsg)
