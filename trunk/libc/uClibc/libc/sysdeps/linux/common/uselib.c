/* vi: set sw=4 ts=4: */
/*
 * uselib() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#ifdef __NR_uselib
_syscall1(int, uselib, const char *, library);
#endif
