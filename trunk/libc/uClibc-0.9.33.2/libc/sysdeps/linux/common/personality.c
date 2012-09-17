/* vi: set sw=4 ts=4: */
/*
 * personality() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/personality.h>
_syscall1(int, personality, unsigned long int, __persona)
