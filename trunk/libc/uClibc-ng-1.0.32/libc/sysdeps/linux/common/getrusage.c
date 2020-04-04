/*
 * getrusage() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <wait.h>
#include <sys/resource.h>
_syscall2(int, getrusage, __rusage_who_t, who, struct rusage *, usage)
