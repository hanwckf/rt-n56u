/* Check alignment of TLS variable.  */
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <tls.h>

#if USE_TLS && HAVE___THREAD

#define AL 4096
struct foo
{
  int i;
} __attribute ((aligned (AL)));

static __thread struct foo f;
static struct foo g;


extern int in_dso1 (void);


static int
do_test (void)
{
  int result = 0;

  int fail = (((uintptr_t) &f) & (AL - 1)) != 0;
  printf ("&f = %p %s\n", &f, fail ? "FAIL" : "OK");
  result |= fail;

  fail = (((uintptr_t) &g) & (AL - 1)) != 0;
  printf ("&g = %p %s\n", &g, fail ? "FAIL" : "OK");
  result |= fail;

  result |= in_dso1 ();

  void *h = dlopen ("tst-tlsmod14b.so", RTLD_LAZY);
  if (h == NULL)
    {
      printf ("cannot open tst-tlsmod14b.so: %m\n");
      exit (1);
    }

  int (*fp) (void) = (int (*) (void)) dlsym (h, "in_dso2");
  if (fp == NULL)
    {
      puts ("cannot find in_dso2");
      exit (1);
    }

  result |= fp ();

  return result;
}

#define TEST_FUNCTION do_test ()

#else

#define TEST_FUNCTION 0

#endif

#include "../test-skeleton.c"
