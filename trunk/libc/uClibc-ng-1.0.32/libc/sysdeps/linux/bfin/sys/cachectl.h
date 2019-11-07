/*
 * cachectl.h - Functions for cache control on Blackfin.
 *
 * Copyright (C) 2010 Analog Devices, Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _SYS_CACHECTL_H
#define _SYS_CACHECTL_H	1

#include <features.h>

/*
 * Get the kernel definition for the flag bits
 */
#include <asm/cachectl.h>

__BEGIN_DECLS

extern int cacheflush (void *addr, const int nbytes, const int flags);

__END_DECLS

#endif	/* sys/cachectl.h */
