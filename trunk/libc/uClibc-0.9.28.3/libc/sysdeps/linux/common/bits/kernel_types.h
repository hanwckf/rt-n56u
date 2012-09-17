#ifndef _BITS_KERNEL_TYPES_H
#define _BITS_KERNEL_TYPES_H

/* Sigh.  We need to carefully wrap this one...  No guarantees
 * that the asm/posix_types.h kernel header is working.  Many
 * arches have broken headers that introduce tons of gratuitous
 * conflicts with uClibc's namespace.   See bits/kernel_types.h
 * for i386, arm, etc for examples... */
#warning You really should include a proper bits/kernel_types.h for your architecture 

#ifndef __GLIBC__
#define __GLIBC__ 2
#include <asm/posix_types.h>
#undef __GLIBC__
#else
#include <asm/posix_types.h>
#endif


#endif /* _BITS_KERNEL_TYPES_H */
