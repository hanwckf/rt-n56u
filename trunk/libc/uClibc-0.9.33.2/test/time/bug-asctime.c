/* Note: we disable this on uClibc because we dont bother
 * verifying if the year is sane ... we just return ????
 * for the year value ...
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>


static int
do_test (void)
{
  int result = 0;
  time_t t = time (NULL);
  struct tm *tp = localtime (&t);
  tp->tm_year = INT_MAX;
  errno = 0;
  char *s = asctime (tp);
  if (s != NULL || errno != EOVERFLOW)
    {
      printf ("asctime did not fail correctly: s=%p, wanted %p; errno=%i, wanted %i\n",
              s, NULL, errno, EOVERFLOW);
      result = 1;
    }
  char buf[1000];
  errno = 0;
  s = asctime_r (tp, buf);
  if (s != NULL || errno != EOVERFLOW)
    {
      printf ("asctime_r did not fail correctly: s=%p, wanted %p; errno=%i, wanted %i\n",
              s, NULL, errno, EOVERFLOW);
      result = 1;
    }
  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
