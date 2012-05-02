/* vi: set sw=4 ts=4: */
/*
 * setpriority() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/resource.h>
_syscall3(int, setpriority, __priority_which_t, which, id_t, who, int, prio);
