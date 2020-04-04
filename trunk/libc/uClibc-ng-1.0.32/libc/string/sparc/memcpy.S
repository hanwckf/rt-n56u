#include "_glibc_inc.h"

#if __WORDSIZE == 32

# if defined(__CONFIG_SPARC_V9B__)
#  include "sparc32/sparcv9b/memcpy.S"
# else
#  include "sparc32/memcpy.S"
# endif

#else

# if defined(__CONFIG_SPARC_V9B__)
#  include "sparc64/sparcv9b/memcpy.S"
# else
#  include "sparc64/memcpy.S"
#endif

#endif
