/* Define the real-function versions of all inline functions
   defined in signal.h (or bits/sigset.h).  */

#include <features.h>

#define _EXTERN_INLINE
#ifndef __USE_EXTERN_INLINES
# define __USE_EXTERN_INLINES	1
#endif

#include <signal.h>

/* Since we massaged signal.h into emitting non-inline function
 * definitions, we need to finish PLT avoidance trick: */
#undef __sigismember
#undef __sigaddset
#undef __sigdelset
libc_hidden_def(__sigismember)
libc_hidden_def(__sigaddset)
libc_hidden_def(__sigdelset)
