/* vi: set sw=4 ts=4: */
/*
 * swapon() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/swap.h>
_syscall2(int, swapon, const char *, path, int, swapflags);
