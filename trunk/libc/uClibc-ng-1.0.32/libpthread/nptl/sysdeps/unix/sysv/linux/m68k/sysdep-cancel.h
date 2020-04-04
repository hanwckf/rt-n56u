#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
#include <pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# ifdef IS_IN_libpthread
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define __local_multiple_threads __libc_multiple_threads
# endif

# if defined IS_IN_libpthread || !defined NOT_IN_libc
extern int __local_multiple_threads attribute_hidden;
#  define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
# else
#  define SINGLE_THREAD_P						\
   __builtin_expect (THREAD_GETMEM (THREAD_SELF,			\
				    header.multiple_threads) == 0, 1)
# endif

#else

# define SINGLE_THREAD_P 1
# define NO_CANCELLATION 1

#endif

#define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
