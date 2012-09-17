#include "tst-tls10.h"

#ifdef USE_TLS__THREAD
__thread int mod15b_var __attribute__((tls_model("initial-exec")));

int
in_dso (void)
{
  return mod15b_var;
}
#else
int
in_dso (void)
{
  return 0;
}
#endif
