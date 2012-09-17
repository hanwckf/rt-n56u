#include <stdio.h>

#ifndef N
# define N 0
#endif

static __thread int var = 4;

int
test (void)
{
  int *p = &var;
  /* GCC assumes &var is never NULL, add optimization barrier.  */
  __asm__ __volatile__ ("" : "+r" (p));
  if (p == NULL || *p != 4)
    {
      printf ("fail %d %p\n", N, p);
      return 1;
    }
  return 0;
}
