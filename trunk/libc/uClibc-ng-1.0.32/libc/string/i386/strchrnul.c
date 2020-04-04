/*
 * Adapted from strchr.c code
 *
 * Copyright (C) 2008 Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <string.h>

#undef strchrnul
/*#define strchrnul TESTING*/
char *strchrnul(const char *s, int c)
{
	int esi;
	char *eax;
	__asm__ __volatile__(
		"	movb	%%al, %%ah\n"
		"1:	lodsb\n"
		"	cmpb	%%ah, %%al\n"
		"	je	2f\n"
		"	testb	%%al, %%al\n"
		"	jnz	1b\n"
		/* with this, we'd get strchr(): */
		/* "	movl	$1, %%esi\n" */
		"2:	leal	-1(%%esi), %%eax\n"
		: "=a" (eax), "=&S" (esi)
		: "0" (c), "1" (s)
		/* no clobbers */
	);
	return eax;
}
#ifndef strchrnul
libc_hidden_def(strchrnul)
#else
/* Uncomment TESTING, gcc -D_GNU_SOURCE -m32 -Os strchrnul.c -o strchrnul
 * and run ./strchrnul
 */
int main()
{
	static const char str[] = "abc.def";
	printf((char*)strchrnul(str, '.') - str == 3 ? "ok\n" : "BAD!\n");
	printf((char*)strchrnul(str, '*') - str == 7 ? "ok\n" : "BAD!\n");
	printf((char*)strchrnul(str,   0) - str == 7 ? "ok\n" : "BAD!\n");
	printf((char*)strchrnul(str+3, '.') - str == 3 ? "ok\n" : "BAD!\n");
}
#endif
