/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/socket.h>
#include <cancel.h>

#ifndef __NR_sendmsg
#error Missing definition of NR_sendmsg needed for cancellation.
#endif

ssize_t
sendmsg (int fd, const struct msghdr *msg, int flags)
{
  return _syscall3(ssize_t, __NC(sendmsg), int, fd, const struct msghdr*, msg,
			int, flags);
}

CANCELLABLE_SYSCALL(ssize_t, sendmsg, (int fd, const struct msghdr *msg,
			int flags), (fd, msg, flags))

lt_libc_hidden(sendmsg)
