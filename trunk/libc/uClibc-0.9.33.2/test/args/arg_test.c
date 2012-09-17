/* vi: set sw=4 ts=4: */
/*
 * Test application for argc and argv handling
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int i=0;
	char** index=__environ;

#ifdef __powerpc__
	{
		unsigned long sp;
		sp = (unsigned long) __builtin_frame_address(0);
		if(sp&0xf){
			printf("stack pointer is unaligned! (%08lx)\n", sp);
		}
	}
#endif

	printf("argc=%d\n", argc);

	for(i=0;i<argc;i++) {
		printf("argv[%d]='%s'\n", i, argv[i]);
	}

	i=0;
	while(*index) {
		printf("environ[%d]='%s'\n", i++, *index++);
	}

	exit(0);
}
