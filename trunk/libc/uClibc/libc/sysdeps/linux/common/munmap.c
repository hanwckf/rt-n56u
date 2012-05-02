/* vi: set sw=4 ts=4: */
/*
 * munmap() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#include <sys/mman.h>
_syscall2(int, munmap, void *, start, size_t, length);
