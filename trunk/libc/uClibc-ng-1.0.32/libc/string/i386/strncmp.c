/*
 * This string-include defines all string functions as inline
 * functions. Use gcc. It also assumes ds=es=data space, this should be
 * normal. Most of the string-functions are rather heavily hand-optimized,
 * see especially strtok,strstr,str[c]spn. They should work, but are not
 * very easy to understand. Everything is done entirely within the register
 * set, making the functions fast and clean. String instructions have been
 * used through-out, making for "slightly" unclear code :-)
 *
 *		NO Copyright (C) 1991, 1992 Linus Torvalds,
 *		consider these trivial functions to be PD.
 */

/*
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*
 * Modified for uClibc by Erik Andersen <andersen@codepoet.org>
 * These make no attempt to use nifty things like mmx/3dnow/etc.
 * These are not inline, and will therefore not be as fast as
 * modifying the headers to use inlines (and cannot therefore
 * do tricky things when dealing with const memory).  But they
 * should (I hope!) be faster than their generic equivalents....
 *
 * More importantly, these should provide a good example for
 * others to follow when adding arch specific optimizations.
 *  -Erik
 */

#include <string.h>

#undef strncmp
int strncmp(const char *cs, const char *ct, size_t count)
{
	int eax;
	int esi, edi, ecx;
	__asm__ __volatile__(
		"	incl	%%ecx\n"
		"1:	decl	%%ecx\n"
		"	jz	2f\n"
		"	lodsb\n"
		"	scasb\n"
		"	jne	3f\n"
		"	testb	%%al, %%al\n"
		"	jnz	1b\n"
		"2:	xorl	%%eax, %%eax\n"
		"	jmp	4f\n"
		"3:	sbbl	%%eax, %%eax\n"
		"	orb	$1, %%al\n"
		"4:\n"
		: "=a" (eax), "=&S" (esi), "=&D" (edi), "=&c" (ecx)
		: "1" (cs), "2" (ct), "3" (count)
	);
	return eax;
}
libc_hidden_weak(strncmp)
