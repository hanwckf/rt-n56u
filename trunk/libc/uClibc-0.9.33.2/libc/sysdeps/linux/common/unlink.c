/* vi: set sw=4 ts=4: */
/*
 * unlink() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>


_syscall1(int, unlink, const char *, pathname)
libc_hidden_def(unlink)
