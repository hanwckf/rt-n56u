#include <tls.h>

#if defined USE_TLS && defined HAVE___THREAD \
    && defined HAVE_TLS_MODEL_ATTRIBUTE
__thread int a[2] __attribute__ ((tls_model ("initial-exec")));
#else
int a[2];
#endif

int
foo (void)
{
  return a[0];
}
