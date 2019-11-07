/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <tls.h>
#include <sysdep.h>
#ifndef __ASSEMBLER__
# include <pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# ifdef IS_IN_libpthread
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define __local_multiple_threads __libc_multiple_threads
# endif

# define SINGLE_THREAD_P	\
    likely(THREAD_GETMEM (THREAD_SELF, header.multiple_threads) == 0)

#endif
