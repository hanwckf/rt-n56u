/* $Id: shmparam.h,v 1.1.1.1 2007-05-25 06:50:14 bruce Exp $ */
#ifndef _ASMSPARC64_SHMPARAM_H
#define _ASMSPARC64_SHMPARAM_H
#ifdef __KERNEL__

#include <asm/spitfire.h>

#define __ARCH_FORCE_SHMLBA	1
/* attach addr a multiple of this */
#define	SHMLBA	((PAGE_SIZE > L1DCACHE_SIZE) ? PAGE_SIZE : L1DCACHE_SIZE)

#endif /* __KERNEL__ */
#endif /* _ASMSPARC64_SHMPARAM_H */
