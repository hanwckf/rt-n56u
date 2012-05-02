/* Tests for pread and pwrite.
   Copyright (C) 1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <search.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>

#define TESTFILE_NAME "CRAP.XXXXXX"
#define STRINGIFY(s) STRINGIFY2 (s)
#define STRINGIFY2(s) #s

/* These are for the temporary file we generate.  */
char *name;
int fd;


/* Test the 32-bit versions first.  */
#define PREAD pread
#define PWRITE pwrite

int test(int argc, char *argv[])
{
    char buf[1000];
    char res[1000];
    int i;

    memset (buf, '\0', sizeof (buf));
    memset (res, '\xff', sizeof (res));

    if (write (fd, buf, sizeof (buf)) != sizeof (buf))
	error (EXIT_FAILURE, errno, "during write");

    for (i = 100; i < 200; ++i)
	buf[i] = i;
    if (PWRITE (fd, buf + 100, 100, 100) != 100)
	error (EXIT_FAILURE, errno, "during %s", STRINGIFY (PWRITE));

    for (i = 450; i < 600; ++i)
	buf[i] = i;
    if (PWRITE (fd, buf + 450, 150, 450) != 150)
	error (EXIT_FAILURE, errno, "during %s", STRINGIFY (PWRITE));

    if (PREAD (fd, res, sizeof (buf) - 50, 50) != sizeof (buf) - 50)
	error (EXIT_FAILURE, errno, "during %s", STRINGIFY (PREAD));

    close (fd);
    unlink (name);
    return memcmp (buf + 50, res, sizeof (buf) - 50);
}

/* Test the 64-bit versions as well.  */
#if defined __UCLIBC_HAS_LFS__ 

#undef PREAD
#undef PWRITE
#define PREAD pread64
#define PWRITE pwrite64


int test64(int argc, char *argv[])
{
    char buf[1000];
    char res[1000];
    int i;

    memset (buf, '\0', sizeof (buf));
    memset (res, '\xff', sizeof (res));

    if (write (fd, buf, sizeof (buf)) != sizeof (buf))
	error (EXIT_FAILURE, errno, "during write");

    for (i = 100; i < 200; ++i)
	buf[i] = i;
    if (PWRITE (fd, buf + 100, 100, 100) != 100)
	error (EXIT_FAILURE, errno, "during %s", STRINGIFY (PWRITE));

    for (i = 450; i < 600; ++i)
	buf[i] = i;
    if (PWRITE (fd, buf + 450, 150, 450) != 150)
	error (EXIT_FAILURE, errno, "during %s", STRINGIFY (PWRITE));

    if (PREAD (fd, res, sizeof (buf) - 50, 50) != sizeof (buf) - 50)
	error (EXIT_FAILURE, errno, "during %s", STRINGIFY (PREAD));

    close (fd);
    unlink (name);
    return memcmp (buf + 50, res, sizeof (buf) - 50);
}
#endif

void prepare(void)
{
    if (!name) {
	name = malloc (BUFSIZ);
	if (name == NULL)
	    error (EXIT_FAILURE, errno, "cannot allocate file name");
    }
    strncpy(name, TESTFILE_NAME, BUFSIZ);

    /* Open our test file.   */
    fd = mkstemp (name);
    if (fd == -1)
	error (EXIT_FAILURE, errno, "cannot open test file `%s'", name);
}

int main (int argc, char **argv)
{
    int result = 0;

    prepare();
    result+=test(argc, argv);
    if (result) { 
	fprintf(stderr, "pread/pwrite test failed.\n");
	return(EXIT_FAILURE);
    }
    fprintf(stderr, "pread/pwrite test successful.\n");

#if defined __UCLIBC_HAS_LFS__ 
    prepare();
    result+=test64(argc, argv);
    if (result) { 
	fprintf(stderr, "pread64/pwrite64 test failed.\n");
	return(EXIT_FAILURE);
    }
    fprintf(stderr, "pread64/pwrite64 test successful.\n");
#endif
    return(EXIT_SUCCESS);
}
