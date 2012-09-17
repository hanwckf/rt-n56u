/* vi: set sw=4 ts=4: */
/*
 * prctl() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <stdarg.h>
//#include <sys/prctl.h>
_syscall5(int, prctl, int, a, int, b, int, c, int, d, int, e);
