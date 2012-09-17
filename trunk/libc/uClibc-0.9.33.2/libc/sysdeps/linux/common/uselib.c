/* vi: set sw=4 ts=4: */
/*
 * uselib() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
#if 0
linux specific and we do not use it in uClibc.

#include <sys/syscall.h>
#include <unistd.h>
#ifdef __NR_uselib
int uselib (const char *library);
_syscall1(int, uselib, const char *, library)
#endif
#endif
