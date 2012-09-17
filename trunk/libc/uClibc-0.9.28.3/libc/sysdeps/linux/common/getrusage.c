/* vi: set sw=4 ts=4: */
/*
 * getrusage() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#include <wait.h>
_syscall2(int, getrusage, int, who, struct rusage *, usage);
