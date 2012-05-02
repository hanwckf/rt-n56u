/* vi: set sw=4 ts=4: */
/*
 * mmap() for uClibc/x86_64
 *
 * Copyright (C) 2005 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2005 by Mike Frysinger <vapier@gentoo.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>

_syscall6(void *, mmap, void *, start, size_t, length, int, prot,
          int, flags, int, fd, off_t, offset);
