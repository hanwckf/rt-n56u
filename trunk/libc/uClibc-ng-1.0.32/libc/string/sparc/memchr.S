#include "_glibc_inc.h"

#if __WORDSIZE == 32

# if defined(__CONFIG_SPARC_V9B__)
#  include "sparc32/sparcv9b/memchr.S"
# else
#  include "sparc32/memchr.S"
# endif

#else
# include "sparc64/memchr.S"
#endif
