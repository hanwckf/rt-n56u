/* Test for fdopen bugs.  */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define assert(x) \
  if (!(x)) \
    { \
      fputs ("test failed: " #x "\n", stderr); \
      retval = 1; \
      goto the_end; \
    }

char buffer[256];

int
main (int argc, char *argv[])
{
  char *name;
  FILE *fp = NULL;
  int retval = 0;
  int fd;

  name = tmpnam (NULL);
  fp = fopen (name, "w");
  assert (fp != NULL)
  fputs ("foobar and baz", fp);
  fclose (fp);
  fp = NULL;

  fd = open (name, O_RDWR|O_CREAT);
  assert (fd != -1);
  assert (lseek (fd, 5, SEEK_SET) == 5);

  fp = fdopen (fd, "a");
  assert (fp != NULL);
  assert (ftell (fp) == 14);

the_end:
  if (fp != NULL)
    fclose (fp);
  unlink (name);

  return retval;
}
