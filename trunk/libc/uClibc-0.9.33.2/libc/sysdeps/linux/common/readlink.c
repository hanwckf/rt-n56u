/* vi: set sw=4 ts=4: */
/*
 * readlink() for uClibc
 *
 * Copyright (C) 2000-2007 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

_syscall3(ssize_t, readlink, const char *, path, char *, buf, size_t, bufsiz)
libc_hidden_def(readlink)
