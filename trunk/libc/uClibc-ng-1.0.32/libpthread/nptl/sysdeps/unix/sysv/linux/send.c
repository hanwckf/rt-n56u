/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/socket.h>
#include <cancel.h>

ssize_t
send (int fd, const void *buf, size_t len, int flags)
{
  return _syscall4(ssize_t, __NC(send), int, fd, const void* buf,
			size_t, len, int, flags);
}

CANCELLABLE_SYSCALL(ssize_t, send, (int fd, const void *buf,
			size_t len, int flags), (fd, buf, len, flags))

lt_libc_hidden(send)
