/* vi: set sw=4 ts=4: */
/*
 * sched_yield() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sched.h>
_syscall0(int, sched_yield);
