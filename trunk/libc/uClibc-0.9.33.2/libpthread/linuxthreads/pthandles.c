#include "pthread.h"
#include "internals.h"

/* Array of active threads. Entry 0 is reserved for the initial thread. */
struct pthread_handle_struct __pthread_handles[PTHREAD_THREADS_MAX];
