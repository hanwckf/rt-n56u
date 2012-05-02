/* vi: set sw=4 ts=4: */
/*
 * getitimer() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/time.h>
_syscall2(int, getitimer, __itimer_which_t, which, struct itimerval *, value);
