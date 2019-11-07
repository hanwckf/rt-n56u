/*
 * Setup some glibc defines so we can just drop in the
 * asm files from glibc without any modification.
 */

#include <features.h>
#include <bits/wordsize.h>

#define ENTRY(sym) \
	.global sym; \
	.type   sym,%function; \
	sym:

#define BP_SYM(sym) sym

#define L(sym) LOC(sym)
#define LOC(sym) \
	.L ## sym

#define END(sym) \
	.size sym,.-sym;
