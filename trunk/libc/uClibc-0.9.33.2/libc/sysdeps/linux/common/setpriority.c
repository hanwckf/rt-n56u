/* vi: set sw=4 ts=4: */
/*
 * setpriority() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/resource.h>


_syscall3(int, setpriority, __priority_which_t, which, id_t, who, int, prio)
libc_hidden_def(setpriority)
