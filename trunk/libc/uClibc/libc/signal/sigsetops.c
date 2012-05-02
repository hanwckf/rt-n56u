/* Define the real-function versions of all inline functions
   defined in signal.h (or bits/sigset.h).  */

#include <features.h>

#define _EXTERN_INLINE
#ifndef __USE_EXTERN_INLINES
# define __USE_EXTERN_INLINES	1
#endif

#include "signal.h"
