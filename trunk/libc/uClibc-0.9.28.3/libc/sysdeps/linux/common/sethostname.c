/* vi: set sw=4 ts=4: */
/*
 * sethostname() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
_syscall2(int, sethostname, const char *, name, size_t, len);
