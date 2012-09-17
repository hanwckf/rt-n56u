#include <locale.h>
#include <mcheck.h>

int
main (void)
{
  int cnt;

  mtrace ();

  for (cnt = 0; cnt < 100; ++cnt)
    {
      setlocale (LC_ALL, "de_DE.ISO-8859-1");
      setlocale (LC_ALL, "de_DE.UTF-8");
    }

  return 0;
}
