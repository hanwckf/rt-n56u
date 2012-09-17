#include <tls.h>

#if defined USE_TLS && defined HAVE___THREAD \
    && defined HAVE_TLS_MODEL_ATTRIBUTE
__thread int b[2] __attribute__ ((tls_model ("initial-exec")));
#else
int b[2];
#endif

extern int foo (void);

int
bar (void)
{
  return foo () + b[0];
}
