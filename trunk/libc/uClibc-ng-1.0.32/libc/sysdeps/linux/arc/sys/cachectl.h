/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#ifndef _SYS_CACHECTL_H
#define _SYS_CACHECTL_H	1

/*
 * Get the kernel definition for the flag bits
 */
#include <asm/cachectl.h>

__BEGIN_DECLS

extern int cacheflush(void *addr, int nbytes, int flags);

__END_DECLS

#endif	/* sys/cachectl.h */
