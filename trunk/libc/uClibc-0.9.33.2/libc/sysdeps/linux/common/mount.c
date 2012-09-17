/* vi: set sw=4 ts=4: */
/*
 * mount() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/mount.h>
_syscall5(int, mount, const char *, specialfile, const char *, dir,
		  const char *, filesystemtype, unsigned long, rwflag,
		  const void *, data)
