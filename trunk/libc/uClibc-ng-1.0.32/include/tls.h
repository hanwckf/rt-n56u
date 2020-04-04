/* This file defines USE___THREAD to 1 or 0 to cut down on the #if mess.  */

#ifndef _include_tls_h
#define _include_tls_h 1

#include_next <tls.h>

#if defined USE_TLS && USE_TLS && HAVE___THREAD \
    && (!defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt)

# define USE___THREAD 1

#else

# define USE___THREAD 0

#endif

#endif
