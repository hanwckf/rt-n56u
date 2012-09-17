/* vi: set sw=4 ts=4: */
/*
 * modify_ldt() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

int modify_ldt (int func, void *ptr, unsigned long bytecount);
#ifdef __NR_modify_ldt
_syscall3(int, modify_ldt, int, func, void *, ptr, unsigned long, bytecount)
#endif
