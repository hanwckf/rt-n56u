/*
 * cacheflush.c - Cache control functions for Blackfin.
 *
 * Copyright (C) 2010 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifdef __NR_cacheflush
# include <sys/cachectl.h>

_syscall3 (int, cacheflush, void *, addr, const int, nbytes, const int, flags)
#endif
