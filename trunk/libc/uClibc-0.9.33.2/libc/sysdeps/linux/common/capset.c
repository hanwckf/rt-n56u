/* vi: set sw=4 ts=4: */
/*
 * capset() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

int capset(void *header, const void *data);
#ifdef __NR_capset
_syscall2(int, capset, void *, header, const void *, data)
#endif
