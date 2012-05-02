/* vi: set sw=4 ts=4: */
/*
 * sched_get_priority_min() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sched.h>
_syscall1(int, sched_get_priority_min, int, policy);
