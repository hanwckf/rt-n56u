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

#undef strncat
/*#define strncat TESTING*/
char *strncat(char * dest, const char * src, size_t count)
{
	int esi, edi, eax, ecx, edx;
	__asm__ __volatile__(
		"	xorl	%%eax, %%eax\n"
		"	incl	%%edx\n"
		"	pushl	%%edi\n" /* save dest */
		"	repne; scasb\n"
		"	decl	%%edi\n" /* edi => NUL in dest */
		/* count-- */
		"1:	decl	%%edx\n"
		/* if count reached 0, store NUL and bail out */
		"	movl	%%edx, %%eax\n"
		"	jz	2f\n"
		/* else copy a char */
		"	lodsb\n"
		"2:	stosb\n"
		"	testb	%%al, %%al\n"
		"	jnz	1b\n"
		/* end of loop */
		"	popl	%%eax\n" /* restore dest into eax */
		: "=&S" (esi), "=&D" (edi), "=&a" (eax), "=&c" (ecx), "=&d" (edx)
		: "0" (src), "1" (dest), "3" (0xffffffff), "4" (count)
		: "memory"
	);
	return (char *)eax;
}
#ifndef strncat
libc_hidden_def(strncat)
#else
/* Uncomment TESTING, gcc -m32 -Os strncat.c -o strncat
 * and run ./strncat
 */
int main()
{
	char buf[99];

	strcpy(buf, "abc"); buf[4] = '*'; strncat(buf, "def", 0);
	printf(strcmp(buf, "abc") == 0 && buf[4] == '*' ? "ok\n" : "BAD!\n");

	strcpy(buf, "abc"); buf[6] = 1; buf[7] = '*'; strncat(buf, "def", 50);
	printf(strcmp(buf, "abcdef") == 0 && buf[7] == '*' ? "ok\n" : "BAD!\n");

	strcpy(buf, "abc"); buf[6] = 1; buf[7] = '*'; strncat(buf, "def", -1);
	printf(strcmp(buf, "abcdef") == 0 && buf[7] == '*' ? "ok\n" : "BAD!\n");

	strcpy(buf, "abc"); buf[6] = 1; buf[7] = '*'; strncat(buf, "def123", 3);
	printf(strcmp(buf, "abcdef") == 0 && buf[7] == '*' ? "ok\n" : "BAD!\n");
}
#endif
