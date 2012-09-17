#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

static int
do_test (void)
{
  void *h = dlopen ("tst-tlsmod15a.so", RTLD_NOW);
  if (h != NULL)
    {
      puts ("unexpectedly succeeded to open tst-tlsmod15a.so");
      exit (1);
    }

  h = dlopen ("tst-tlsmod15b.so", RTLD_NOW);
  if (h == NULL)
    {
      puts ("failed to open tst-tlsmod15b.so");
      exit (1);
    }

  int (*fp) (void) = (int (*) (void)) dlsym (h, "in_dso");
  if (fp == NULL)
    {
      puts ("cannot find in_dso");
      exit (1);
    }

  return fp ();
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
