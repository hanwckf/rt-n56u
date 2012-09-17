#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include <link.h>
#include <tls.h>

#define TEST_FUNCTION do_test ()
static int
do_test (void)
{
#ifdef USE_TLS
  static const char modname1[] = "tst-tlsmod5.so";
  static const char modname2[] = "tst-tlsmod6.so";
  int result = 0;

  void *h1 = dlopen (modname1, RTLD_LAZY);
  if (h1 == NULL)
    {
      printf ("cannot open '%s': %s\n", modname1, dlerror ());
      result = 1;
    }
  void *h2 = dlopen (modname2, RTLD_LAZY);
  if (h2 == NULL)
    {
      printf ("cannot open '%s': %s\n", modname2, dlerror ());
      result = 1;
    }

  if (h1 != NULL)
    dlclose (h1);
  if (h2 != NULL)
    dlclose (h2);

  return result;
#else
  return 0;
#endif
}


#include "../test-skeleton.c"
