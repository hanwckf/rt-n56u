/* vi: set sw=4 ts=4: */
/*
 * fork test for uClibc
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

int main(void) 
{
	char *foo;
	char junk[12];
	char crap[100];
	foo = getcwd(NULL, 0);
	printf("getcwd(NULL, 0)='%s'\n", foo);
	if (foo) { free(foo); }
	foo = getcwd(NULL, 100);
	printf("\ngetcwd(NULL, 100)='%s'\n", foo);
	if (foo) { free(foo); }
	foo = getcwd(junk, sizeof(junk));
	printf("\nchar junk[12];\n");
	printf("getcwd(junk, sizeof(junk))='%s'\n", foo);
	foo = getcwd(crap, sizeof(crap));
	printf("\nchar crap[100];\n");
	printf("getcwd(crap, sizeof(crap))='%s'\n", foo);
	return EXIT_SUCCESS;
}

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
