/*
 *
 */

#include <features.h>
#include <errno.h>
#include <netdb.h>

#ifndef __UCLIBC_HAS_TLS__

#undef errno
#undef h_errno

extern int h_errno;
extern int errno;

#ifdef __UCLIBC_HAS_THREADS__
libc_hidden_proto(h_errno)
libc_hidden_proto(errno)
#endif
#endif
