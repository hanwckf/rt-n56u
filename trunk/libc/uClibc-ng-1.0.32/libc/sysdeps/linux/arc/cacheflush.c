/* cacheflush syscall for ARC
 *
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/cachectl.h>

_syscall3(int, cacheflush, void *, addr, int, nbytes, int, op)
