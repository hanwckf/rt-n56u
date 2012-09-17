/* vi: set sw=4 ts=4: */
/*
 * mmap() for uClibc/x86_64
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 * Copyright (C) 2005 by Mike Frysinger <vapier@gentoo.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>


_syscall6(void *, mmap, void *, start, size_t, length, int, prot,
          int, flags, int, fd, off_t, offset)
libc_hidden_def(mmap)
