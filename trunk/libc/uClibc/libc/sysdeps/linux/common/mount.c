/* vi: set sw=4 ts=4: */
/*
 * mount() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/mount.h>
_syscall5(int, mount, const char *, specialfile, const char *, dir,
		  const char *, filesystemtype, unsigned long, rwflag,
		  const void *, data);
