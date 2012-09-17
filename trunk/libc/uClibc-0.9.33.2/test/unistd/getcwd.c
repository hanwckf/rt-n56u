/* vi: set sw=4 ts=4: */
/*
 * fork test for uClibc
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
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
