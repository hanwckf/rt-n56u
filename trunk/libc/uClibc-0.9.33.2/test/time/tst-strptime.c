/* Test for strptime.
   Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
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

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


static const struct
{
  const char *locale;
  const char *input;
  const char *format;
  int wday;
  int yday;
  int mon;
  int mday;
} day_tests[] =
{
  { "C", "2000-01-01", "%Y-%m-%d", 6, 0, 0, 1 },
  { "C", "03/03/00", "%D", 5, 62, 2, 3 },
  { "C", "9/9/99", "%x", 4, 251, 8, 9 },
  { "C", "19990502123412", "%Y%m%d%H%M%S", 0, 121, 4, 2 },
  { "C", "2001 20 Mon", "%Y %U %a", 1, 140, 4, 21 },
  { "C", "2001 21 Mon", "%Y %W %a", 1, 140, 4, 21 },
  { "ja_JP.EUC-JP", "2000-01-01 08:12:21 AM", "%Y-%m-%d %I:%M:%S %p",
    6, 0, 0, 1 },
  { "en_US.ISO-8859-1", "2000-01-01 08:12:21 PM", "%Y-%m-%d %I:%M:%S %p",
    6, 0, 0, 1 },
  { "ja_JP.EUC-JP", "2001 20 \xb7\xee", "%Y %U %a", 1, 140, 4, 21 },
  { "ja_JP.EUC-JP", "2001 21 \xb7\xee", "%Y %W %a", 1, 140, 4, 21 },
};


static const struct
{
  const char *input;
  const char *format;
  const char *output;
  int wday;
  int yday;
} tm_tests [] =
{
  {"17410105012000", "%H%M%S%d%m%Y", "2000-01-05 17:41:01", 3, 4}
};



static int
test_tm (void)
{
  struct tm tm;
  size_t i;
  int result = 0;
  char buf[100];

  for (i = 0; i < sizeof (tm_tests) / sizeof (tm_tests[0]); ++i)
    {
      memset (&tm, '\0', sizeof (tm));

      char *ret = strptime (tm_tests[i].input, tm_tests[i].format, &tm);
      if (ret == NULL)
	{
	  printf ("strptime returned NULL for `%s'\n", tm_tests[i].input);
	  result = 1;
	  continue;
	}
      else if (*ret != '\0')
	{
	  printf ("not all of `%s' read\n", tm_tests[i].input);
	  result = 1;
	}
      strftime (buf, sizeof (buf), "%F %T", &tm);
      printf ("strptime (\"%s\", \"%s\", ...)\n"
	      "\tshould be: %s, wday = %d, yday = %3d\n"
	      "\t       is: %s, wday = %d, yday = %3d\n",
	      tm_tests[i].input, tm_tests[i].format,
	      tm_tests[i].output,
	      tm_tests[i].wday, tm_tests[i].yday,
	      buf, tm.tm_wday, tm.tm_yday);

      if (strcmp (buf, tm_tests[i].output) != 0)
	{
	  printf ("Time and date are not correct.\n");
	  result = 1;
	}
      if (tm.tm_wday != tm_tests[i].wday)
	{
	  printf ("weekday for `%s' incorrect: %d instead of %d\n",
		  tm_tests[i].input, tm.tm_wday, tm_tests[i].wday);
	  result = 1;
	}
      if (tm.tm_yday != tm_tests[i].yday)
	{
	  printf ("yearday for `%s' incorrect: %d instead of %d\n",
		  tm_tests[i].input, tm.tm_yday, tm_tests[i].yday);
	  result = 1;
	}
    }

  return result;
}


int
main (int argc, char *argv[])
{
  struct tm tm;
  size_t i;
  int result = 0;

  for (i = 0; i < sizeof (day_tests) / sizeof (day_tests[0]); ++i)
    {
      memset (&tm, '\0', sizeof (tm));

      if (setlocale (LC_ALL, day_tests[i].locale) == NULL)
	{
	  printf ("cannot set locale %s: %m\n", day_tests[i].locale);
	  exit (EXIT_FAILURE);
	}

      char *ret = strptime (day_tests[i].input, day_tests[i].format, &tm);
      if (ret == NULL)
	{
	  printf ("strptime returned NULL for `%s'\n", day_tests[i].input);
	  result = 1;
	  continue;
	}
      else if (*ret != '\0')
	{
	  printf ("not all of `%s' read\n", day_tests[i].input);
	  result = 1;
	}

      printf ("strptime (\"%s\", \"%s\", ...)\n"
	      "\tshould be: wday = %d, yday = %3d, mon = %2d, mday = %2d\n"
	      "\t       is: wday = %d, yday = %3d, mon = %2d, mday = %2d\n",
	      day_tests[i].input, day_tests[i].format,
	      day_tests[i].wday, day_tests[i].yday,
	      day_tests[i].mon, day_tests[i].mday,
	      tm.tm_wday, tm.tm_yday, tm.tm_mon, tm.tm_mday);

      if (tm.tm_wday != day_tests[i].wday)
	{
	  printf ("weekday for `%s' incorrect: %d instead of %d\n",
		  day_tests[i].input, tm.tm_wday, day_tests[i].wday);
	  result = 1;
	}
      if (tm.tm_yday != day_tests[i].yday)
	{
	  printf ("yearday for `%s' incorrect: %d instead of %d\n",
		  day_tests[i].input, tm.tm_yday, day_tests[i].yday);
	  result = 1;
	}
      if (tm.tm_mon != day_tests[i].mon)
	{
	  printf ("month for `%s' incorrect: %d instead of %d\n",
		  day_tests[i].input, tm.tm_mon, day_tests[i].mon);
	  result = 1;
	}
      if (tm.tm_mday != day_tests[i].mday)
	{
	  printf ("monthday for `%s' incorrect: %d instead of %d\n",
		  day_tests[i].input, tm.tm_mday, day_tests[i].mday);
	  result = 1;
	}
    }

  setlocale (LC_ALL, "C");

  result |= test_tm ();

  return result;
}
