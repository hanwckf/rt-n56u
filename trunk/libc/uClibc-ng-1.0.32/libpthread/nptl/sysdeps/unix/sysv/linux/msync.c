/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/mman.h>
#include <cancel.h>

int
msync (void *addr, size_t length, int flags)
{
  return _syscall3(int, __NC(msync), const void* addr,
			size_t, lenght, int, flags);
}

CANCELLABLE_SYSCALL(int, msync, (const void *addr,
			size_t lenght, int flags), (addr, length, flags))

lt_libc_hidden(msync)
