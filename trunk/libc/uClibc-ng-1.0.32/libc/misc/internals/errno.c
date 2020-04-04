#include <features.h>
#include <errno.h>
#undef errno

#ifdef __UCLIBC_HAS_TLS__
__thread int errno attribute_tls_model_ie;
#else
extern int errno;
int errno = 0;
# ifdef __UCLIBC_HAS_THREADS__
strong_alias(errno,_errno)
# endif
#endif
