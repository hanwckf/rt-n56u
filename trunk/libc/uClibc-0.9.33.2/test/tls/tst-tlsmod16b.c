#include <tls.h>

#if defined HAVE___THREAD && defined HAVE_TLS_MODEL_ATTRIBUTE
extern __thread int tlsvar __attribute__((tls_model("initial-exec")));
#else
extern int tlsvar;
#endif

void *
in_dso (void)
{
  return &tlsvar;
}
