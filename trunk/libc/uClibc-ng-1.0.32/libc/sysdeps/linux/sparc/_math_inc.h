/*
 * Setup some glibc defines so we can just drop in the
 * asm files from glibc without any modification.
 */

#include <features.h>

#define __ASSEMBLY__
#include <asm/traps.h>

/* Is alignment really needed? */

#define ENTRY_ALIGN 4

#define ENTRY(sym) \
	.global sym; \
	.align  ENTRY_ALIGN; \
	.type   sym,%function; \
	sym:

#define LOC(sym) \
	.L ## sym

#define END(sym) \
	.size sym,.-sym;
