/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <sys/syscall.h>
_syscall3(int, cacheflush, void *, addr, int, nbytes, int, op)
