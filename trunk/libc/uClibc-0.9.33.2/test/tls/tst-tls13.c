/* Check unloading modules with data in static TLS block.  */
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>


static int
do_test (void)
{
  int i;
  for (i = 0; i < 1000;)
    {
      printf ("round %d\n",++i);

      void *h = dlopen ("tst-tlsmod13a.so", RTLD_LAZY);
      if (h == NULL)
	{
	  printf ("cannot load: %s\n", dlerror ());
	  exit (1);
	}

      dlclose (h);
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#define TIMEOUT 20
#include "../test-skeleton.c"
