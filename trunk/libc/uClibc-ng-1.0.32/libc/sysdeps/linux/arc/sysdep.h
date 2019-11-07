#ifndef _LINUX_ARC_SYSDEP_H
#define _LINUX_ARC_SYSDEP_H 1

#include <features.h>
#include <libc-internal.h>

#ifdef	__ASSEMBLER__

#define ENTRY(nm)		\
	.text `			\
	.align 4 `		\
	.globl nm `		\
	.type nm,@function `	\
nm:

#define END(name)	.size name,.-name

#endif /* __ASSEMBLER __*/

#include <common/sysdep.h>
#endif
