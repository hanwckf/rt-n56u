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

char *strrchr(const char *s, int c)
{
	char *eax;

	__asm__ __volatile__(
		"	movb	%%cl, %%ch\n"
		"1:	movb	(%1), %%cl\n" /* load char */
		"	cmpb	%%cl, %%ch\n" /* char == c? */
		"	jne	2f\n"
		"	movl	%1, %%eax\n"
		"2:	incl	%1\n"
		"	testb	%%cl, %%cl\n" /* char == NUL? */
		"	jnz	1b\n"
		/* "=c": use ecx, not ebx (-fpic uses it). */
		: "=a" (eax), "=r" (s), "=c" (c)
		: "0" (0), "1" (s), "2" (c)
		/* : no clobbers */
	);
	return eax;
}
libc_hidden_def(strrchr)
#ifdef __UCLIBC_SUSV3_LEGACY__
strong_alias(strrchr,rindex)
#endif
