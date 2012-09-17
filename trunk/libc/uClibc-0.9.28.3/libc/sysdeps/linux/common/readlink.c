/* vi: set sw=4 ts=4: */
/*
 * readlink() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
_syscall3(int, readlink, const char *, path, char *, buf, size_t, bufsiz);
