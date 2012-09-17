#include <features.h>

#ifdef __UCLIBC_HAS_TLS__
__thread int errno;
__thread int h_errno;

extern __thread int __libc_errno __attribute__ ((alias ("errno"))) attribute_hidden;
extern __thread int __libc_h_errno __attribute__ ((alias ("h_errno"))) attribute_hidden;
#define h_errno __libc_h_errno

#else
#include "internal_errno.h"
int errno = 0;
int h_errno = 0;
#ifdef __UCLIBC_HAS_THREADS__
libc_hidden_def(errno)
weak_alias(errno, _errno)
libc_hidden_def(h_errno)
weak_alias(h_errno, _h_errno)
#endif
#endif
