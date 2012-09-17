#include <stdio.h>
#include <time.h>
#include <features.h>
#ifdef __UCLIBC_HAS_WCHAR__
#include <wchar.h>

int
main (int argc, char *argv[])
{
  wchar_t buf[200];
  time_t t;
  struct tm *tp;
  int result = 0;
  size_t n;

  time (&t);
  tp = gmtime (&t);

  n = wcsftime (buf, sizeof (buf) / sizeof (buf[0]),
		L"%H:%M:%S  %Y-%m-%d\n", tp);
  if (n != 21)
    result = 1;

  wprintf (L"It is now %ls", buf);

  wcsftime (buf, sizeof (buf) / sizeof (buf[0]), L"%A\n", tp);

  wprintf (L"The weekday is %ls", buf);

  return result;
}

#else
int main(void)
{
	puts("Test requires WCHAR support; skipping");
	return 0;
}
#endif
