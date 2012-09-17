#include <tls.h>

#if defined HAVE___THREAD && defined HAVE_TLS_MODEL_ATTRIBUTE
int __thread tlsvar;
#else
int tlsvar;
#endif
