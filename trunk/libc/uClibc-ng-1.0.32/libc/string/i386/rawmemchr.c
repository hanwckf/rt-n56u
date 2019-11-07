/*
 * Adapted from strlen.c code
 *
 * Copyright (C) 2008 Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <string.h>

#undef rawmemchr
void *rawmemchr(const void *s, int c)
{
	void *eax;
	int ecx, edi;
	__asm__ __volatile__(
		"	repne; scasb\n"
		"	leal	-1(%%edi), %%eax\n"
		: "=&c" (ecx), "=&D" (edi), "=&a" (eax)
		: "0" (0xffffffff), "1" (s), "2" (c)
	);
	return eax;
}
libc_hidden_def(rawmemchr)
