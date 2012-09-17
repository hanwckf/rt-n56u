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

#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>


/* Allow testing of the 64-bit versions as well.  */
#ifndef PREAD
# define PREAD pread
# define PWRITE pwrite
#endif

#define STRINGIFY(s) STRINGIFY2 (s)
#define STRINGIFY2(s) #s

/* Prototype for our test function.  */
extern void do_prepare (int argc, char *argv[]);
extern int do_test (int argc, char *argv[]);

/* We have a preparation function.  */
#define PREPARE do_prepare

/* We might need a bit longer timeout.  */
#define TIMEOUT 20 /* sec */

/* This defines the `main' function and some more.  */
#include "../test-skeleton.c"

/* These are for the temporary file we generate.  */
char *name;
int fd;

void
do_prepare (int argc, char *argv[])
{
   char name_len;

#define FNAME FNAME2(TRUNCATE)
#define FNAME2(s) "/" STRINGIFY(s) "XXXXXX"

   name_len = strlen (test_dir);
   name = malloc (name_len + sizeof (FNAME));
   if (name == NULL)
     error (EXIT_FAILURE, errno, "cannot allocate file name");
   mempcpy (mempcpy (name, test_dir, name_len), FNAME, sizeof (FNAME));
   add_temp_file (name);

   /* Open our test file.   */
   fd = mkstemp (name);
   if (fd == -1)
     error (EXIT_FAILURE, errno, "cannot open test file `%s'", name);
}


int
do_test (int argc, char *argv[])
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
