#include <limits.h>
#include <stdio.h>
#include <time.h>


static const struct
{
  const char *fmt;
  long int gmtoff;
} tests[] =
  {
    { "1113472456 +1000", 36000 },
    { "1113472456 -1000", -36000 },
    { "1113472456 +10", 36000 },
    { "1113472456 -10", -36000 },
    { "1113472456 +1030", 37800 },
    { "1113472456 -1030", -37800 },
    { "1113472456 +0030", 1800 },
    { "1113472456 -0030", -1800 },
    { "1113472456 -1330", LONG_MAX },
    { "1113472456 +1330", LONG_MAX },
    { "1113472456 -1060", LONG_MAX },
    { "1113472456 +1060", LONG_MAX },
    { "1113472456  1030", LONG_MAX },
  };
#define ntests (sizeof (tests) / sizeof (tests[0]))


int
main (void)
{
  int result = 0;

  for (int i = 0; i < ntests; ++i)
    {
      struct tm tm;

      if (strptime (tests[i].fmt, "%s %z", &tm) == NULL)
	{
	  if (tests[i].gmtoff != LONG_MAX)
	    {
	      printf ("round %d: strptime unexpectedly failed\n", i);
	      result = 1;
	    }
	  continue;
	}

      if (tm.tm_gmtoff != tests[i].gmtoff)
	{
	  printf ("round %d: tm_gmtoff is %ld\n", i, (long int) tm.tm_gmtoff);
	  result = 1;
	}
    }

  if (result == 0)
    puts ("all OK");

  return 0;
}
