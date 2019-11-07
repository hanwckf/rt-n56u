/*
 * symlinkat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_symlinkat
_syscall3(int, symlinkat, const char *, from, int, tofd, const char *, to)
libc_hidden_def(symlinkat)
#else
/* should add emulation with symlink() and /proc/self/fd/ ... */
#endif
