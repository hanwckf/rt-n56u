/* Test program from Paul Eggert and Tony Leneis.  */
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

static time_t time_t_max;
static time_t time_t_min;

/* Values we'll use to set the TZ environment variable.  */
static const char *tz_strings[] =
  {
    (const char *) 0, "GMT0", "JST-9",
    "EST+3EDT+2,M10.1.0/00:00:00,M2.3.0/00:00:00"
  };
#define N_STRINGS ((int) (sizeof (tz_strings) / sizeof (tz_strings[0])))

/* Fail if mktime fails to convert a date in the spring-forward gap.
   Based on a problem report from Andreas Jaeger.  */
static void
spring_forward_gap (void)
{
  /* glibc (up to about 1998-10-07) failed this test. */
  struct tm tm;

  /* Use the portable POSIX.1 specification "TZ=PST8PDT,M4.1.0,M10.5.0"
     instead of "TZ=America/Vancouver" in order to detect the bug even
     on systems that don't support the Olson extension, or don't have the
     full zoneinfo tables installed.  */
  setenv ("TZ", "PST8PDT,M4.1.0,M10.5.0", 1);

  tm.tm_year = 98;
  tm.tm_mon = 3;
  tm.tm_mday = 5;
  tm.tm_hour = 2;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  if (mktime (&tm) == (time_t)-1)
    exit (1);
}

static void
mktime_test1 (time_t now)
{
  struct tm *lt = localtime (&now);
  if (lt && mktime (lt) != now)
    exit (2);
}

static void
mktime_test (time_t now)
{
  mktime_test1 (now);
  mktime_test1 ((time_t) (time_t_max - now));
  mktime_test1 ((time_t) (time_t_min + now));
}

static void
irix_6_4_bug (void)
{
  /* Based on code from Ariel Faigon.  */
  struct tm tm;
  tm.tm_year = 96;
  tm.tm_mon = 3;
  tm.tm_mday = 0;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  mktime (&tm);
  if (tm.tm_mon != 2 || tm.tm_mday != 31)
    exit (3);
}

static void
bigtime_test (int j)
{
  struct tm tm;
  time_t now;
  tm.tm_year = tm.tm_mon = tm.tm_mday = tm.tm_hour = tm.tm_min = tm.tm_sec = j;
  tm.tm_isdst = -1;
  now = mktime (&tm);
  if (now != (time_t) -1)
    {
      struct tm *lt = localtime (&now);
      if (! (lt
	     && lt->tm_year == tm.tm_year
	     && lt->tm_mon == tm.tm_mon
	     && lt->tm_mday == tm.tm_mday
	     && lt->tm_hour == tm.tm_hour
	     && lt->tm_min == tm.tm_min
	     && lt->tm_sec == tm.tm_sec
	     && lt->tm_yday == tm.tm_yday
	     && lt->tm_wday == tm.tm_wday
	     && ((lt->tm_isdst < 0 ? -1 : 0 < lt->tm_isdst)
		  == (tm.tm_isdst < 0 ? -1 : 0 < tm.tm_isdst))))
	exit (4);
    }
}

static int
do_test (void)
{
  time_t t, delta;
  int i, j;

  setenv ("TZ", "America/Sao_Paulo", 1);
  /* This test makes some buggy mktime implementations loop.
     Give up after 60 seconds; a mktime slower than that
     isn't worth using anyway.  */
  alarm (60);

  for (time_t_max = 1; 0 < time_t_max; time_t_max *= 2)
    continue;
  time_t_max--;
  if ((time_t) -1 < 0)
    for (time_t_min = -1; (time_t) (time_t_min * 2) < 0; time_t_min *= 2)
      continue;
  delta = time_t_max / 997; /* a suitable prime number */
  for (i = 0; i < N_STRINGS; i++)
    {
      if (tz_strings[i])
	setenv ("TZ", tz_strings[i], 1);

      for (t = 0; t <= time_t_max - delta; t += delta)
	mktime_test (t);
      mktime_test ((time_t) 1);
      mktime_test ((time_t) (60 * 60));
      mktime_test ((time_t) (60 * 60 * 24));

      for (j = 1; 0 < j; j *= 2)
	bigtime_test (j);
      bigtime_test (j - 1);
    }
  irix_6_4_bug ();
  spring_forward_gap ();
  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
