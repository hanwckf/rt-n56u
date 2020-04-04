/*
 * getrandom() for uClibc
 *
 * Copyright (C) 2015 Bernhard Reutner-Fischer
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/random.h>
#ifdef __NR_getrandom
_syscall3(int, getrandom, void *, buf, size_t, buflen, unsigned int, flags)
#endif
