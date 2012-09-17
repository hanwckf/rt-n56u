/* Test for fdopen bugs.  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define assert(x) \
  if (!(x)) \
    { \
      fputs ("test failed: " #x "\n", stderr); \
      retval = 1; \
      goto the_end; \
    }

int
main (int argc, char *argv[])
{
  char name[256];
  FILE *fp = NULL;
  int retval = 0;
  int fd;

  /* hack to get a tempfile name w/out using tmpname()
   * as that func causes a link time warning */
  sprintf(name, "%s-uClibc-test.XXXXXX", __FILE__);
  fd = mkstemp(name);
  close(fd);

  fp = fopen (name, "w");
  assert (fp != NULL)
  assert (fputs ("foobar and baz", fp) > 0);
  assert (fclose (fp) == 0);
  fp = NULL;

  fd = open (name, O_RDWR|O_CREAT, 0660);
  assert (fd != -1);
  assert (lseek (fd, 5, SEEK_SET) == 5);

  fp = fdopen (fd, "a");
  assert (fp != NULL);
  /* SuSv3 says that doing a fdopen() does not reset the file position,
   * thus the '5' here is correct, not '14'. */
  assert (ftell (fp) == 5);

the_end:
  if (fp != NULL)
    assert (fclose (fp) == 0);
  unlink (name);

  return retval;
}
