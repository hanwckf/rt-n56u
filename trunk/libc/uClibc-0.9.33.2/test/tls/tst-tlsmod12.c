#include "tst-tls10.h"

#ifdef USE_TLS__THREAD
extern __thread struct A a2 __attribute__((tls_model("initial-exec")));

void
check1 (void)
{
  if (a1.a != 4 || a1.b != 5 || a1.c != 6)
    abort ();
  if (a2.a != 7 || a2.b != 8 || a2.c != 9)
    abort ();
}
#endif
