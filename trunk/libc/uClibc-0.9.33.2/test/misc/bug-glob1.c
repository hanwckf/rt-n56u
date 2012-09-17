/* Test case for globbing dangling symlink.  By Ulrich Drepper.  */
#include <errno.h>
#include <error.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static void prepare (int argc, char *argv[]);
#define PREPARE prepare
static int do_test (void);
#define TEST_FUNCTION do_test ()

#include "../test-skeleton.c"


static char *fname;

static void
prepare (int argc, char *argv[])
{
  if (argc < 2)
    error (EXIT_FAILURE, 0, "missing argument");

  size_t len = strlen (argv[1]);
  static const char ext[] = "globXXXXXX";
  fname = malloc (len + sizeof (ext));
  if (fname == NULL)
    error (EXIT_FAILURE, errno, "cannot create temp file");
 again:
  strcpy (stpcpy (fname, argv[1]), ext);

/* fname = mktemp (fname); */
  close(mkstemp(fname));
  unlink(fname);

  if (fname == NULL || *fname == '\0')
    error (EXIT_FAILURE, errno, "cannot create temp file name");
  if (symlink ("bug-glob1-does-not-exist", fname) != 0)
    {
      if (errno == EEXIST)
	goto again;

      error (EXIT_FAILURE, errno, "cannot create symlink");
    }
  add_temp_file (fname);
}


static int
do_test (void)
{
  glob_t gl;
  int retval = 0;
  int e;

  e = glob (fname, 0, NULL, &gl);
  if (e == 0)
    {
      printf ("glob(\"%s\") succeeded when it should not have\n", fname);
      retval = 1;
    }
  globfree (&gl);

  size_t fnamelen = strlen (fname);
  char buf[fnamelen + 2];

  strcpy (buf, fname);
  buf[fnamelen - 1] = '?';
  e = glob (buf, 0, NULL, &gl);
  if (e == 0)
    {
      printf ("glob(\"%s\") succeeded when it should not have\n", buf);
      retval = 1;
    }
  globfree (&gl);

  strcpy (buf, fname);
  buf[fnamelen] = '*';
  buf[fnamelen + 1] = '\0';
  e = glob (buf, 0, NULL, &gl);
  if (e == 0)
    {
      printf ("glob(\"%s\") succeeded when it should not have\n", buf);
      retval = 1;
    }
  globfree (&gl);

  return retval;
}
