#include <features.h>
#include <netdb.h>
#undef h_errno

#ifdef __UCLIBC_HAS_TLS__
__thread int h_errno;
extern __thread int __libc_h_errno __attribute__ ((alias ("h_errno"))) attribute_hidden;
#else
extern int h_errno;
int h_errno = 0;
# ifdef __UCLIBC_HAS_THREADS__
strong_alias(h_errno,_h_errno)
# endif
#endif
