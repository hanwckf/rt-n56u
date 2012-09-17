/* vi: set sw=4 ts=4: */
/*
 * sigaltstack() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <signal.h>
_syscall2(int, sigaltstack, const struct sigaltstack *, ss,
		  struct sigaltstack *, oss);
