/* vi: set sw=4 ts=4: */
/*
 * times() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/times.h>
_syscall1(clock_t, times, struct tms *, buf);
