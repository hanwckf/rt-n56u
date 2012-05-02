/* vi: set sw=4 ts=4: */
/*
 * Test application for argc and argv handling
 *
 * Copyright (C) 2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 2000,2001 by Erik Andersen <andersen@uclibc.org>
 * Written by Erik Andersen <andersen@uclibc.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
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
