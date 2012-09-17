/* vi: set sw=4 ts=4: */
/*
 * capget() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
int capget(void *header, void *data);
#ifdef __NR_capget
_syscall2(int, capget, void *, header, void *, data)
#endif
