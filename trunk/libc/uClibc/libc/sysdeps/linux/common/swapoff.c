/* vi: set sw=4 ts=4: */
/*
 * swapoff() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/swap.h>
_syscall1(int, swapoff, const char *, path);
