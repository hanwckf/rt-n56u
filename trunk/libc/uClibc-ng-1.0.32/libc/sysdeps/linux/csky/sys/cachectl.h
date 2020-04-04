/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifndef _SYS_CACHECTL_H
#define _SYS_CACHECTL_H	1

#include <asm/cachectl.h>

__BEGIN_DECLS
extern int cacheflush(void *addr, int nbytes, int flags);
__END_DECLS

#endif
